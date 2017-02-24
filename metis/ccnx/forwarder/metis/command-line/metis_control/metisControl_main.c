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

#include <config.h>

#include <stdbool.h>
#include <stdint.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

#include <LongBow/runtime.h>
#include <string.h>

#include <parc/security/parc_Security.h>
#include <parc/security/parc_IdentityFile.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/algol/parc_List.h>
#include <parc/algol/parc_ArrayList.h>

#include <ccnx/api/control/cpi_ManageLinks.h>

#include <ccnx/api/control/cpi_Forwarding.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/core/metis_Dispatcher.h>

#include <ccnx/api/ccnx_Portal/ccnx_Portal.h>
#include <ccnx/api/ccnx_Portal/ccnx_PortalRTA.h>

#include <ccnx/common/ccnx_KeystoreUtilities.h>
#include <ccnx/transport/common/transport_MetaMessage.h>

#include <ccnx/forwarder/metis/metis_About.h>
#include <ccnx/forwarder/metis/config/metis_ControlState.h>
#include <ccnx/forwarder/metis/config/metisControl_Root.h>

#include <errno.h>

typedef struct metis_control_main_state {
    KeystoreParams *keystoreParams;
    CCNxPortal *controlPortal;
    MetisControlState *controlState;
} MetisControlMainState;

static void
_displayForwarderLogo(void)
{
    printf("%s\n", metisAbout_About());

    printf("            __  __        _    _\n");
    printf("           |  \\/  |  ___ | |_ (_) ___\n");
    printf("           | |\\/| | / _ \\| __|| |/ __|\n");
    printf("           | |  | ||  __/| |_ | |\\__ \\\n");
    printf("           |_|  |_| \\___| \\__||_||___/\n");

    printf("\n");
}

static void
_displayUsage(char *programName)
{
    printf("Usage: %s -h\n", programName);
    printf("       %s [--k|--keystore <keystore file name>] [--p|--password <keystore password>] [commands]\n", programName);
    printf("\n");
    printf("Metis is the CCNx 1.0 forwarder, which runs on each end system and as a software forwarder\n");
    printf("on intermediate systems.  metis_control is the program to configure the forwarder, metis_daemon.\n");
    printf("\n");
    printf("Options:\n");
    printf("-h              = This help screen\n");
    printf("-k | --keystore = Specify the path of the PKCS12 keystore (default ~/.ccnx/.ccnx_keystore.p12)\n");
    printf("-p | --password = keystore password (default to prompt user)\n");
    printf("commands        = configuration line to send to metis (use 'help' for list)\n");
    printf("\n");
}

static int
_parseArgs(int argc, char *argv[], char **keystorePath, char **keystorePassword, PARCList *commandList)
{
    static struct option longFormOptions[] = {
        { "help",     no_argument,       0, 'h' },
        { "keystore", required_argument, 0, 'k' },
        { "password", required_argument, 0, 'p' },
        { 0,          0,                 0, 0   }
    };

    int c;

    while (1) {
        // getopt_long stores the option index here.
        int optionIndex = 0;

        c = getopt_long(argc, argv, "hk:p:", longFormOptions, &optionIndex);

        // Detect the end of the options.
        if (c == -1) {
            break;
        }

        switch (c) {
            case 'k':
                *keystorePath = optarg;
                break;

            case 'p':
                *keystorePassword = optarg;
                break;

            case 'h':
            default:
                _displayUsage(argv[0]);
                return 0;
        }
    }

    // Any remaining parameters get put in the command list.
    if (optind < argc) {
        while (optind < argc) {
            parcList_Add(commandList, argv[optind]);
            optind++;
        }
    }

    return 1;
}

static CCNxMetaMessage *
_writeAndReadMessage(void *mainStatePtr, CCNxMetaMessage *msg)
{
    assertNotNull(mainStatePtr, "Parameter mainStatePtr must be non-null");
    assertNotNull(msg, "Parameter msg must be non-null");

    MetisControlMainState *mainState = mainStatePtr;

    if (ccnxPortal_Send(mainState->controlPortal, msg, CCNxStackTimeout_Never)) {
        CCNxMetaMessage *result = ccnxPortal_Receive(mainState->controlPortal, CCNxStackTimeout_Never);
        assertTrue(result != NULL, "Error reading response from Portal: (%d) %s\nb", errno, strerror(errno));

        return result;
    }

    return NULL;
}

static CCNxPortal *
_createPortalWithKeystore(const char *keystoreName, const char *keystorePassword)
{
    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);
    PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);
    CCNxPortalFactory *portalFactory = ccnxPortalFactory_Create(identity);

    CCNxPortal *result = ccnxPortalFactory_CreatePortal(portalFactory, ccnxPortalRTA_Message);

    ccnxPortalFactory_Release(&portalFactory);
    parcIdentity_Release(&identity);
    parcIdentityFile_Release(&identityFile);

    return result;
}

static MetisControlMainState
_openKeyStore(char *keystorePath, char *keystorePassword)
{
    MetisControlMainState mainState;

    if (keystorePassword == NULL) {
        keystorePassword = ccnxKeystoreUtilities_ReadPassword();

        mainState.keystoreParams = ccnxKeystoreUtilities_OpenFile(keystorePath, keystorePassword);
        parcMemory_Deallocate((void *) &keystorePassword);
    } else {
        mainState.keystoreParams = ccnxKeystoreUtilities_OpenFile(keystorePath, keystorePassword);
    }

    return mainState;
}

int
main(int argc, char *argv[])
{
    _displayForwarderLogo();

    if (argc == 2 && strcmp("-h", argv[1]) == 0) {
        _displayUsage(argv[0]);
        exit(EXIT_SUCCESS);
    }

    char *keystorePath = NULL;
    char *keystorePassword = NULL;

    PARCList *commands = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);

    // Extract optional keystore and password, and optional commands.
    if (!_parseArgs(argc, argv, &keystorePath, &keystorePassword, commands)) {
        parcList_Release(&commands);
        exit(EXIT_FAILURE);
    }

    if (keystorePath == NULL) {
        printf("No keystore specified. Will try default.\n");
    } else {
        printf("Using keystore: %s\n", keystorePath);
    }

    parcSecurity_Init();

    MetisControlMainState mainState = _openKeyStore(keystorePath, keystorePassword);
    if (mainState.keystoreParams == NULL) {
        printf("Could not open keystore '%s'\n", keystorePath == NULL ? "~/.ccnx/.ccnx_keystore.p12" : keystorePath);
        exit(EXIT_FAILURE);
    }

    mainState.controlPortal = _createPortalWithKeystore(ccnxKeystoreUtilities_GetFileName(mainState.keystoreParams),
                                                        ccnxKeystoreUtilities_GetPassword(mainState.keystoreParams));
    parcSecurity_Fini();

    mainState.controlState = metisControlState_Create(&mainState, _writeAndReadMessage);

    metisControlState_RegisterCommand(mainState.controlState, metisControlRoot_HelpCreate(mainState.controlState));
    metisControlState_RegisterCommand(mainState.controlState, metisControlRoot_Create(mainState.controlState));

    if (parcList_Size(commands) > 0) {
        metisControlState_DispatchCommand(mainState.controlState, commands);
    } else {
        metisControlState_Interactive(mainState.controlState);
    }

    parcList_Release(&commands);

    metisControlState_Destroy(&mainState.controlState);

    keystoreParams_Destroy(&mainState.keystoreParams);

    ccnxPortal_Release(&mainState.controlPortal);

    return EXIT_SUCCESS;
}
