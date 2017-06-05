/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>

#include <getopt.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Object.h>

#include <parc/security/parc_Security.h>

#include <ccnx/common/ccnx_Name.h>

#include <ccnx/api/ccnx_Portal/ccnx_Portal.h>
#include <ccnx/api/ccnx_Portal/ccnx_PortalRTA.h>

#include "iPing_Common.h"

typedef struct ccnx_ping_server {
  CCNxPortal *portal;
  CCNxName *prefix;
  size_t payloadSize;

  uint8_t generalPayload[ccnxPing_MaxPayloadSize];
} CCNxPingServer;

/**
 * Create a new CCNxPortalFactory instance using a randomly generated identity saved to
 * the specified keystore.
 *
 * @return A new CCNxPortalFactory instance which must eventually be released by calling ccnxPortalFactory_Release().
 */
static CCNxPortalFactory *_setupServerPortalFactory(void) {
  const char *keystoreName = "server.keystore";
  const char *keystorePassword = "keystore_password";
  const char *subjectName = "server";

  return ccnxPingCommon_SetupPortalFactory(keystoreName, keystorePassword, subjectName);
}

/**
 * Release the references held by the `CCNxPingClient`.
 */
static bool _ccnxPingServer_Destructor(CCNxPingServer **serverPtr) {
  CCNxPingServer *server = *serverPtr;
  if (server->portal != NULL) {
    ccnxPortal_Release(&(server->portal));
  }
  if (server->prefix != NULL) {
    ccnxName_Release(&(server->prefix));
  }
  return true;
}

parcObject_Override(CCNxPingServer, PARCObject, .destructor = (PARCObjectDestructor *) _ccnxPingServer_Destructor);

parcObject_ImplementAcquire(ccnxPingServer, CCNxPingServer);
parcObject_ImplementRelease(ccnxPingServer, CCNxPingServer);

/**
 * Create a new empty `CCNxPingServer` instance.
 */
static CCNxPingServer *ccnxPingServer_Create(void) {
  CCNxPingServer *server = parcObject_CreateInstance(CCNxPingServer);

  server->prefix = ccnxName_CreateFromCString(ccnxPing_DefaultPrefix);
  server->payloadSize = ccnxPing_DefaultPayloadSize;

  return server;
}

/**
 * Create a `PARCBuffer` payload of the server-configured size.
 */
PARCBuffer *_ccnxPingServer_MakePayload(CCNxPingServer *server, int size) {
  PARCBuffer *payload = parcBuffer_Wrap(server->generalPayload, size, 0, size);
  return payload;
}

/**
 * Run the `CCNxPingServer` indefinitely.
 */
static void _ccnxPingServer_Run(CCNxPingServer *server) {
  CCNxPortalFactory *factory = _setupServerPortalFactory();
  server->portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_Message);
  ccnxPortalFactory_Release(&factory);

  size_t yearInSeconds = 60 * 60 * 24 * 365;

  size_t sizeIndex;

  if (ccnxPortal_Listen(server->portal, server->prefix, yearInSeconds, CCNxStackTimeout_Never)) {
    while (true) {
      CCNxMetaMessage *request = ccnxPortal_Receive(server->portal, CCNxStackTimeout_Never);

      // This should never happen.
      if (request == NULL) {
        break;
      }

      CCNxInterest *interest = ccnxMetaMessage_GetInterest(request);
      if (interest != NULL) {
        CCNxName *interestName = ccnxInterest_GetName(interest);

        // Extract the size of the payload response from the client
        sizeIndex = ccnxName_GetSegmentCount(interestName) - 2;
        CCNxNameSegment *sizeSegment = ccnxName_GetSegment(interestName, sizeIndex);
        char *segmentString = ccnxNameSegment_ToString(sizeSegment);
        int size = atoi(segmentString);
        size = size > ccnxPing_MaxPayloadSize ? ccnxPing_MaxPayloadSize : size;

        PARCBuffer *payload = _ccnxPingServer_MakePayload(server, size);

        CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(interestName, payload);
        CCNxMetaMessage *message = ccnxMetaMessage_CreateFromContentObject(contentObject);

        if (ccnxPortal_Send(server->portal, message, CCNxStackTimeout_Never) == false) {
          fprintf(stderr, "ccnxPortal_Send failed: %d\n", ccnxPortal_GetError(server->portal));
        }

        parcMemory_Deallocate(&segmentString);
        ccnxContentObject_Release(&contentObject);
        ccnxMetaMessage_Release(&message);
        parcBuffer_Release(&payload);

      }
      ccnxMetaMessage_Release(&request);
    }
  }
}

/**
 * Display the usage message.
 */
static void _displayUsage(char *progName) {
  printf("CCNx Simple Ping Performance Test\n");
  printf("\n");
  printf("Usage: %s [-l locator] [-s size] \n", progName);
  printf("       %s -h\n", progName);
  printf("\n");
  printf("Example:\n");
  printf("    ccnxPing_Server -l ccnx:/some/prefix -s 1400\n");
  printf("\n");
  printf("Options:\n");
  printf("     -h (--help) Show this help message\n");
  printf("     -l (--locator) Set the locator for this server. The default is 'ccnx:/locator'. \n");
  printf(
      "     -s (--size) Set the payload size (less than 1400 - see `ccnxPing_MaxPayloadSize` in ccnxPing_Common.h)\n");
}

/**
 * Parse the command lines to initialize the state of the
 */
static bool _ccnxPingServer_ParseCommandline(CCNxPingServer *server, int argc, char *argv[argc]) {
  static struct option longopts[] =
      {{"locator", required_argument, NULL, 'l'}, {"size", required_argument, NULL, 's'},
       {"help", no_argument, NULL, 'h'
       }, {NULL, 0, NULL, 0}};

  // Default value
  server->payloadSize = ccnxPing_MaxPayloadSize;

  int c;
  while ((c = getopt_long(argc, argv, "l:s:h", longopts, NULL)) != -1) {
    switch (c) {
      case 'l':
        server->prefix = ccnxName_CreateFromCString(optarg);
        break;
      case 's':
        sscanf(optarg, "%zu", &(server->payloadSize));
        if (server->payloadSize > ccnxPing_MaxPayloadSize) {
          _displayUsage(argv[0]);
          return false;
        }
        break;
      case 'h':
        _displayUsage(argv[0]);
        return false;
      default:
        break;
    }
  }

  return true;
};

int main(int argc, char *argv[argc]) {
  parcSecurity_Init();

  CCNxPingServer *server = ccnxPingServer_Create();
  bool runServer = _ccnxPingServer_ParseCommandline(server, argc, argv);

  if (runServer) {
    _ccnxPingServer_Run(server);
  }

  ccnxPingServer_Release(&server);

  parcSecurity_Fini();

  return EXIT_SUCCESS;
}
