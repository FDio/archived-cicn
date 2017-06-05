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
#include <inttypes.h>
#include <getopt.h>
#include <LongBow/runtime.h>
#include <ccnx/api/ccnx_Portal/ccnx_Portal.h>
#include <ccnx/api/ccnx_Portal/ccnx_PortalRTA.h>
#include <parc/algol/parc_Clock.h>
#include <parc/security/parc_Security.h>
#include <parc/algol/parc_DisplayIndented.h>
#include "iPing_Stats.h"
#include "iPing_Common.h"

typedef enum {
  CCNxPingClientMode_None = 0, CCNxPingClientMode_Flood, CCNxPingClientMode_PingPong, CCNxPingClientMode_All
} CCNxPingClientMode;

typedef struct ccnx_Ping_client {
  CCNxPortal *portal;
  CCNxPortalFactory *factory;
  CCNxPingStats *stats;
  CCNxPingClientMode mode;

  CCNxName *prefix;

  size_t numberOfOutstanding;
  uint64_t receiveTimeoutInMs;
  uint32_t interestLifetime;
  int interestCounter;
  size_t count;
  uint64_t intervalInMs;
  size_t payloadSize;
  int nonce;
} CCNxPingClient;

/**
 * Create a new CCNxPortalFactory instance using a randomly generated identity saved to
 * the specified keystore.
 *
 * @return A new CCNxPortalFactory instance which must eventually be released by calling ccnxPortalFactory_Release().
 */
static CCNxPortalFactory *_setupClientPortalFactory(void) {
  const char *keystoreName = "client.keystore";
  const char *keystorePassword = "keystore_password";
  const char *subjectName = "client";

  return ccnxPingCommon_SetupPortalFactory(keystoreName, keystorePassword, subjectName);
}

/**
 * Release the references held by the `CCNxPingClient`.
 */
static bool _ccnxPingClient_Destructor(CCNxPingClient **clientPtr) {
  CCNxPingClient *client = *clientPtr;

  if (client->factory != NULL) {
    ccnxPortalFactory_Release(&(client->factory));
  }
  if (client->portal != NULL) {
    ccnxPortal_Release(&(client->portal));
  }
  if (client->prefix != NULL) {
    ccnxName_Release(&(client->prefix));
  }
  if (client->stats != NULL) {
    ccnxPingStats_Release(&client->stats);
  }
  return true;
}

parcObject_Override(CCNxPingClient, PARCObject, .destructor = (PARCObjectDestructor *) _ccnxPingClient_Destructor);

parcObject_ImplementAcquire(ccnxPingClient, CCNxPingClient);
parcObject_ImplementRelease(ccnxPingClient, CCNxPingClient);

/**
 * Create a new empty `CCNxPingClient` instance.
 */
static CCNxPingClient *ccnxPingClient_Create(void) {
  CCNxPingClient *client = parcObject_CreateInstance(CCNxPingClient);

  client->stats = ccnxPingStats_Create();
  client->interestCounter = 100;
  client->prefix = ccnxName_CreateFromCString(ccnxPing_DefaultPrefix);
  client->receiveTimeoutInMs = ccnxPing_DefaultReceiveTimeoutInUs / 1000;
  client->count = 10;
  client->intervalInMs = 1000;
  client->nonce = rand();
  client->numberOfOutstanding = 0;
  client->interestLifetime = 4 * 1000; //4s

  return client;
}

/**
 * Get the next `CCNxName` to issue. Increment the interest counter
 * for the client.
 */
static CCNxName *_ccnxPingClient_CreateNextName(CCNxPingClient *client) {
  client->interestCounter++;
  char *suffixBuffer = NULL;

  asprintf(&suffixBuffer, "%x", client->nonce);
  CCNxName *name1 = ccnxName_ComposeNAME(client->prefix, suffixBuffer);
  free(suffixBuffer);

  suffixBuffer = NULL;
  asprintf(&suffixBuffer, "%u", (unsigned int) client->payloadSize);
  CCNxName *name2 = ccnxName_ComposeNAME(name1, suffixBuffer);
  free(suffixBuffer);
  ccnxName_Release(&name1);

  suffixBuffer = NULL;
  asprintf(&suffixBuffer, "%06lu", (long) client->interestCounter);
  CCNxName *name3 = ccnxName_ComposeNAME(name2, suffixBuffer);
  free(suffixBuffer);
  ccnxName_Release(&name2);

  return name3;
}

/**
 * Convert a timeval struct to a single microsecond count.
 */
static uint64_t _ccnxPingClient_CurrentTimeInUs(PARCClock *clock) {
  struct timeval currentTimeVal;
  parcClock_GetTimeval(clock, &currentTimeVal);
  uint64_t microseconds = ((uint64_t) currentTimeVal.tv_sec) * 1000000 + currentTimeVal.tv_usec;
  return microseconds;
}

/**
 * Run a single ping test.
 */
static void _ccnxPingClient_RunPing(CCNxPingClient *client, size_t totalPings) {
  uint64_t delayInUs = client->intervalInMs * 1000;
  uint64_t timeoutInUs = client->receiveTimeoutInMs * 1000;
  PARCClock *clock = parcClock_Wallclock();

  client->factory = _setupClientPortalFactory();
  client->portal = ccnxPortalFactory_CreatePortal(client->factory, ccnxPortalRTA_Message);

  size_t outstanding = 0;

  uint64_t nextPacketSendTime = 0;
  uint64_t currentTimeInUs = 0;

  for (int pings = 0; pings < totalPings; pings++) {

    CCNxName *name;
    CCNxInterest *interest;
    CCNxMetaMessage *message;

    name = _ccnxPingClient_CreateNextName(client);
    interest = ccnxInterest_CreateSimple(name);
    message = ccnxMetaMessage_CreateFromInterest(interest);
    ccnxInterest_SetLifetime(interest, client->interestLifetime);

    if (ccnxPortal_Send(client->portal, message, CCNxStackTimeout_Never)) {
      currentTimeInUs = _ccnxPingClient_CurrentTimeInUs(clock);
      nextPacketSendTime = currentTimeInUs + delayInUs;

      ccnxPingStats_RecordRequest(client->stats, name, currentTimeInUs);
    } else {
      printf("Error in sending\n");
      nextPacketSendTime = 0;
    }

    ccnxMetaMessage_Release(&message);
    ccnxInterest_Release(&interest);
    outstanding++;

    /* Now wait for the responses and record their times
     * First packet wait until its arrives or timeout
     */
    currentTimeInUs = _ccnxPingClient_CurrentTimeInUs(clock);
    uint64_t receiveDelay = timeoutInUs;
    CCNxMetaMessage *response = ccnxPortal_Receive(client->portal, &receiveDelay);

    //Remove the timeout ping only if we are in the ping mode
    if (response == NULL && (client->mode != CCNxPingClientMode_Flood)) {
      ccnxPingStats_RecordLost(client->stats, name);
      outstanding--;
    }
    ccnxName_Release(&name);

    /* Loop to handle content that arrives too late, that is after receiveDelay
     * They will be just evicted from the hashtable and not considered in the
     */
    while (response != NULL) {
      printf("Received content\n");
      currentTimeInUs = _ccnxPingClient_CurrentTimeInUs(clock);
      if (ccnxMetaMessage_IsContentObject(response)) {
        CCNxContentObject *contentObject = ccnxMetaMessage_GetContentObject(response);
        CCNxName *responseName = ccnxContentObject_GetName(contentObject);
        bool existing;
        uint64_t
            delta = ccnxPingStats_RecordResponse(client->stats, responseName, currentTimeInUs, response, &existing);
        // Only display output if we're in ping mode
        if (client->mode == CCNxPingClientMode_PingPong && existing) {
          size_t contentSize = parcBuffer_Remaining(ccnxContentObject_GetPayload(contentObject));
          char *nameString = ccnxName_ToString(responseName);
          printf("%zu bytes from %s: time=%"PRIu64" us\n", contentSize, nameString, delta);
          parcMemory_Deallocate(&nameString);
        }
      }
      ccnxMetaMessage_Release(&response);

      outstanding--;

      currentTimeInUs = _ccnxPingClient_CurrentTimeInUs(clock);

      /* The following covers the case in which a content arrived after
       * receiveDelay. If that happens, it will be read from the portal
       * after the next interest sending, therefore we might have two
       * contents to read rather than one. This code is fine as long as we
       * do not consider in statistics content arriving after receiveDelay
       */
      receiveDelay = nextPacketSendTime - currentTimeInUs;
      response = ccnxPortal_Receive(client->portal, &receiveDelay);
    }
  }

  /* We're done with pings, so let's wait to see if we have any
   * stragglers. We wait for 30s more
   */
  currentTimeInUs = _ccnxPingClient_CurrentTimeInUs(clock);
  nextPacketSendTime = currentTimeInUs + 10000000; //10s

  uint64_t receiveDelay = nextPacketSendTime - currentTimeInUs;
  CCNxMetaMessage *response = ccnxPortal_Receive(client->portal, &receiveDelay);

  /* Loop to handle content that arrives too late, that is after receiveDelay
   * They will be just evicted from the hashtable and not considered in the
   */
  while (response != NULL) {
    printf("Received content\n");
    currentTimeInUs = _ccnxPingClient_CurrentTimeInUs(clock);
    if (ccnxMetaMessage_IsContentObject(response)) {
      CCNxContentObject *contentObject = ccnxMetaMessage_GetContentObject(response);
      CCNxName *responseName = ccnxContentObject_GetName(contentObject);
      bool existing;
      uint64_t delta = ccnxPingStats_RecordResponse(client->stats, responseName, currentTimeInUs, response, &existing);
      // Only display output if we're in ping mode
      if (client->mode == CCNxPingClientMode_PingPong && existing) {
        size_t contentSize = parcBuffer_Remaining(ccnxContentObject_GetPayload(contentObject));
        char *nameString = ccnxName_ToString(responseName);
        printf("%zu bytes from %s: time=%"PRIu64" us\n", contentSize, nameString, delta);
        parcMemory_Deallocate(&nameString);
      }
    }
    ccnxMetaMessage_Release(&response);
    outstanding--;
    currentTimeInUs = _ccnxPingClient_CurrentTimeInUs(clock);
    /* The following covers the case in which a content arrived after
     * receiveDelay. If that happens, it will be read from the portal
     * after the next interest sending, therefore we might have two
     * contents to read rather than one. This code is fine as long as we
     * do not consider in statistics content arriving after receiveDelay
     */
    receiveDelay = nextPacketSendTime - currentTimeInUs;
    response = ccnxPortal_Receive(client->portal, &receiveDelay);
  }
  parcClock_Release(&clock);
}

/**
 * Display the usage message.
 */
static void _displayUsage(char *progName) {
  printf("CCNx Simple Ping Performance Test\n");
  printf("   (you must have ccnxPing_Server running)\n");
  printf("\n");
  printf("Usage: %s -p [ -c count ] [ -s size ] [ -i interval ]\n", progName);
  printf("       %s -f [ -c count ] [ -s size ]\n", progName);
  printf("       %s -h\n", progName);
  printf("\n");
  printf("Example:\n");
  printf("    ccnxPing_Client -l ccnx:/some/prefix -c 100 -f\n");
  printf("\n");
  printf("Options:\n");
  printf("     -h (--help) Show this help message\n");
  printf("     -p (--ping) ping mode - \n");
  printf(
      "     -f (--flood) flood mode - send as fast as possible. ATTENTION, NO MIGHT USE A LOT OF MEMORY IF THE NUMBER OF SENDING INTEREST IS HUGE\n");
  printf("     -c (--count) Number of count to run\n");
  printf(
      "     -i (--interval) Interval in milliseconds between interests in ping mode. Such interval cannot always be satisfied because in ping mode the application waits to receive a content before issuing the next interest. If any content is received interval==timeout\n");
  printf("     -s (--size) Size of the interests\n");
  printf("     -l (--locator) Set the locator for this server. The default is 'ccnx:/locator'. \n");
  printf(
      "     -t (--timeout) Time that the application waits for a content. When elapsed the content will be dropped and RTT not considered. Default timeout==1s\n");
  printf(
      "     -e (--lifetime) Set interest lifetime in milliseconds. When elapsed the interest is evicted only from the PIT. Eviction from the application internal state (used for recording interest sending time adn calculate RTT). Default lifetime==4s.\n");
}

/**
 * Parse the command lines to initialize the state of the
 */
static bool _ccnxPingClient_ParseCommandline(CCNxPingClient *client, int argc, char *argv[argc]) {
  static struct option longopts[] =
      {{"ping", no_argument, NULL, 'p'}, {"flood", no_argument, NULL, 'f'}, {"count", required_argument, NULL, 'c'}, {
          "size", required_argument, NULL, 's'
      }, {"interval", required_argument, NULL, 'i'}, {"locator", required_argument, NULL, 'l'},
       {"outstanding", required_argument, NULL, 'o'
       }, {"help", no_argument, NULL, 'h'}, {"timeout", required_argument, NULL, 't'},
       {"lifetime", required_argument, NULL, 'e'
       }, {NULL, 0, NULL, 0}};

  client->payloadSize = ccnxPing_DefaultPayloadSize;

  int c;
  while ((c = getopt_long(argc, argv, "phfc:s:i:l:o:t:e:", longopts, NULL)) != -1) {
    switch (c) {
      case 'p':
        if (client->mode != CCNxPingClientMode_None) {
          return false;
        }
        client->mode = CCNxPingClientMode_PingPong;
        break;
      case 'f':
        if (client->mode != CCNxPingClientMode_None) {
          return false;
        }
        client->mode = CCNxPingClientMode_Flood;
        client->intervalInMs = 0;
        client->receiveTimeoutInMs = 0;
        break;
      case 'c':
        sscanf(optarg, "%zu", &(client->count));
        break;
      case 'i':
        if (client->mode != CCNxPingClientMode_Flood) {
          sscanf(optarg, "%"PRIu64"", &(client->intervalInMs));
          printf("Timer %"PRIu64"\n", (client->intervalInMs));
        }
        break;
      case 't':
        if (client->mode != CCNxPingClientMode_Flood) {
          sscanf(optarg, "%"PRIu64"", &(client->receiveTimeoutInMs));
        }
        break;
      case 's':
        sscanf(optarg, "%zu", &(client->payloadSize));
        break;
      case 'o':
        sscanf(optarg, "%zu", &(client->numberOfOutstanding));
        break;
      case 'l':
        ccnxName_Release(&(client->prefix));
        client->prefix = ccnxName_CreateFromCString(optarg);
        break;
      case 'h':
        _displayUsage(argv[0]);
        return false;
      case 'e':
        sscanf(optarg, "%"PRIu32"", &(client->interestLifetime));
        break;
      default:
        break;
    }
  }
  if (client->mode == CCNxPingClientMode_None) {
    _displayUsage(argv[0]);
    return false;
  }
  return true;
};

static void _ccnxPingClient_DisplayStatistics(CCNxPingClient *client) {
  bool ableToCompute = ccnxPingStats_Display(client->stats);
  if (!ableToCompute) {
    parcDisplayIndented_PrintLine(0,
                                  "No packets were received. Check to make sure the client and server are configured correctly and that the forwarder is running.\n");
  }
}

static void _ccnxPingClient_RunPingormanceTest(CCNxPingClient *client) {
  switch (client->mode) {
    case CCNxPingClientMode_All:
      _ccnxPingClient_RunPing(client, mediumNumberOfPings);
      _ccnxPingClient_DisplayStatistics(client);

      ccnxPingStats_Release(&client->stats);
      client->stats = ccnxPingStats_Create();

      _ccnxPingClient_RunPing(client, smallNumberOfPings);
      _ccnxPingClient_DisplayStatistics(client);
      break;
    case CCNxPingClientMode_Flood:
      _ccnxPingClient_RunPing(client, client->count);
      _ccnxPingClient_DisplayStatistics(client);
      break;
    case CCNxPingClientMode_PingPong:
      _ccnxPingClient_RunPing(client, client->count);
      _ccnxPingClient_DisplayStatistics(client);
      break;
    case CCNxPingClientMode_None:
    default:
      fprintf(stderr, "Error, unknown mode");
      break;
  }
}

int main(int argc, char *argv[argc]) {
  parcSecurity_Init();
  CCNxPingClient *client = ccnxPingClient_Create();
  bool runPing = _ccnxPingClient_ParseCommandline(client, argc, argv);
  if (runPing) {
    _ccnxPingClient_RunPingormanceTest(client);
  }
  ccnxPingClient_Release(&client);
  parcSecurity_Fini();
  return EXIT_SUCCESS;
}
