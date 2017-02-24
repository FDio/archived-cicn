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

/*
 * NB: binds to all interfaces on the listen port, which might be a security issue.
 *
 * The CLI runs as an event managed listener.  The Api here creates, starts, stops, and destroys it.
 *
 * The CLI is a user interface to the programmatic interface in <code>metis_Configuration.h</code>
 *
 */

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_ArrayList.h>

#include "metis_CommandLineInterface.h"

struct metis_command_line_interface {
    MetisForwarder *metis;
    PARCEventSocket *listener;
    PARCArrayList *openSessions;

    uint16_t port;
};

typedef struct metis_cli_session {
    MetisCommandLineInterface *parentCli;
    MetisSocketType clientSocket;
    struct sockaddr *clientAddress;
    int clientAddressLength;
    PARCEventQueue *streamBuffer;
    bool doingTheRightThing;
} _MetisCommandLineInterface_Session;

struct metis_cli_command;
typedef struct metis_cli_command _MetisCommandLineInterface_Command;

struct metis_cli_command {
    char *text;
    char *helpDescription;
    void (*func)(_MetisCommandLineInterface_Session *session, _MetisCommandLineInterface_Command *command, const char *params);
};

static void _metisCommandLineInterface_ListenerCallback(MetisSocketType client_socket,
                                                        struct sockaddr *client_addr, int socklen, void *user_data);

static _MetisCommandLineInterface_Session *metisCliSession_Create(MetisCommandLineInterface *cli, MetisSocketType client_socket, struct sockaddr *client_addr, int socklen);
static void _metisCliSession_Destory(_MetisCommandLineInterface_Session **cliSessionPtr);
static void _metisCliSession_ReadCallback(PARCEventQueue *event, PARCEventType type, void *cliSessionVoid);
static void _metisCliSession_EventCallback(PARCEventQueue *event, PARCEventQueueEventType what, void *cliSessionVoid);
static bool _metisCliSession_ProcessCommand(_MetisCommandLineInterface_Session *session, char *cmdline);
static void _metisCliSession_DisplayMotd(_MetisCommandLineInterface_Session *session);
static void _metisCliSession_DisplayPrompt(_MetisCommandLineInterface_Session *session);

// used by PARCArrayList
static void
_session_VoidDestroyer(void **cliSessionVoidPtr)
{
    _MetisCommandLineInterface_Session **cliSessionPtr = (_MetisCommandLineInterface_Session **) cliSessionVoidPtr;
    (*cliSessionPtr)->doingTheRightThing = true;
    _metisCliSession_Destory(cliSessionPtr);
}

// ====================================================================================

static void _cmd_Help(_MetisCommandLineInterface_Session *session, _MetisCommandLineInterface_Command *command, const char *params);
static void _cmd_Show(_MetisCommandLineInterface_Session *session, _MetisCommandLineInterface_Command *command, const char *params);
static void _cmd_Exit(_MetisCommandLineInterface_Session *session, _MetisCommandLineInterface_Command *command, const char *params);
static void _cmd_Tunnel(_MetisCommandLineInterface_Session *session, _MetisCommandLineInterface_Command *command, const char *params);
static void _cmd_Version(_MetisCommandLineInterface_Session *session, _MetisCommandLineInterface_Command *command, const char *params);

/**
 * @typedef _cliCommands
 * The commands, their short help, and their function pointer
 * @constant <#name#> <#description#>
 *  List must be terminated with a NULL entry
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static _MetisCommandLineInterface_Command _cliCommands[] = {
    { "exit",   "Ends the session",       _cmd_Exit    },
    { "help",   "Displays the help menu", _cmd_Help    },
    { "show",   "Displays state",         _cmd_Show    },
    { "tunnel", "manage tunnels",         _cmd_Tunnel  },
    { "ver",    "Forwarder version",      _cmd_Version },
    { NULL,     NULL,                     NULL         }
};

// ====================================================================================

MetisCommandLineInterface *
metisCommandLineInterface_Create(MetisForwarder *metis, uint16_t port)
{
    MetisCommandLineInterface *cli = parcMemory_AllocateAndClear(sizeof(MetisCommandLineInterface));
    assertNotNull(cli, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisCommandLineInterface));
    cli->port = port;
    cli->listener = NULL;
    cli->metis = metis;
    cli->openSessions = parcArrayList_Create(_session_VoidDestroyer);

    return cli;
}

void
metisCommandLineInterface_Start(MetisCommandLineInterface *cli)
{
    // listen address
    struct sockaddr_in6 addr6;
    memset(&addr6, 0, sizeof(addr6));
    addr6.sin6_family = PF_INET6;
    addr6.sin6_port = htons(cli->port);

    MetisDispatcher *dispatcher = metisForwarder_GetDispatcher(cli->metis);
    PARCEventSocket *listener = metisDispatcher_CreateListener(dispatcher, _metisCommandLineInterface_ListenerCallback, cli, -1, (struct sockaddr *) &addr6, sizeof(addr6));
    assertNotNull(listener, "Got null listener");

    cli->listener = listener;
}

void
metisCommandLineInterface_Destroy(MetisCommandLineInterface **cliPtr)
{
    assertNotNull(cliPtr, "Parameter must be non-null double pointer");
    assertNotNull(*cliPtr, "Parameter must dereference to non-null pointer");

    MetisCommandLineInterface *cli = *cliPtr;

    parcArrayList_Destroy(&cli->openSessions);

    if (cli->listener) {
        MetisDispatcher *dispatcher = metisForwarder_GetDispatcher(cli->metis);
        metisDispatcher_DestroyListener(dispatcher, &(cli->listener));
    }

    parcMemory_Deallocate((void **) &cli);
    *cliPtr = NULL;
}

/**
 * Creates a client-specific session
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static _MetisCommandLineInterface_Session *
metisCliSession_Create(MetisCommandLineInterface *cli, MetisSocketType clientSocket, struct sockaddr *clientAddress, int clientAddressLength)
{
    _MetisCommandLineInterface_Session *session = parcMemory_AllocateAndClear(sizeof(_MetisCommandLineInterface_Session));
    assertNotNull(session, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(_MetisCommandLineInterface_Session));
    session->parentCli = cli;
    session->clientAddress = parcMemory_Allocate(clientAddressLength);
    assertNotNull(session->clientAddress, "parcMemory_Allocate(%d) returned NULL", clientAddressLength);
    session->clientAddressLength = clientAddressLength;
    session->clientSocket = clientSocket;

    memcpy(session->clientAddress, clientAddress, clientAddressLength);

    MetisDispatcher *dispatcher = metisForwarder_GetDispatcher(cli->metis);
    PARCEventScheduler *eventBase = metisDispatcher_GetEventScheduler(dispatcher);
    session->streamBuffer = parcEventQueue_Create(eventBase, clientSocket, PARCEventQueueOption_CloseOnFree | PARCEventQueueOption_DeferCallbacks);

    parcEventQueue_SetCallbacks(session->streamBuffer, _metisCliSession_ReadCallback, NULL, _metisCliSession_EventCallback, session);
    parcEventQueue_Enable(session->streamBuffer, PARCEventType_Read);

    return session;
}

/**
 * SHOULD ONLY BE CALLED FROM ARRAYLIST
 *
 *   Do not call this on your own!!  It should only be called when an
 *   item is removed from the cli->openSessions array list.
 *
 *   Will close the tcp session and free memory.
 *
 * @param <#param1#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
_metisCliSession_Destory(_MetisCommandLineInterface_Session **cliSessionPtr)
{
    assertNotNull(cliSessionPtr, "Parameter must be non-null double pointer");
    assertNotNull(*cliSessionPtr, "Parameter must dereference to non-null pointer");
    _MetisCommandLineInterface_Session *session = *cliSessionPtr;

    assertTrue(session->doingTheRightThing, "Ha! caught you!  You called Destroy outside the PARCArrayList");

    parcEventQueue_Destroy(&(session->streamBuffer));
    parcMemory_Deallocate((void **) &(session->clientAddress));
    parcMemory_Deallocate((void **) &session);
    *cliSessionPtr = NULL;
}

/**
 * Called on a new connection to the server socket
 *
 *   Will allocate a new _MetisCommandLineInterface_Session and put it in the
 *   server's PARCArrayList
 *
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void
_metisCommandLineInterface_ListenerCallback(MetisSocketType client_socket,
                                            struct sockaddr *client_addr, int socklen, void *user_data)
{
    MetisCommandLineInterface *cli = (MetisCommandLineInterface *) user_data;
    _MetisCommandLineInterface_Session *session = metisCliSession_Create(cli, client_socket, client_addr, socklen);
    parcArrayList_Add(cli->openSessions, session);

    _metisCliSession_DisplayMotd(session);
    _metisCliSession_DisplayPrompt(session);
}

static void
_metisCliSession_ReadCallback(PARCEventQueue *event, PARCEventType type, void *cliSessionVoid)
{
    assertTrue(type == PARCEventType_Read, "Illegal type: expected read event, got %d\n", type);
    _MetisCommandLineInterface_Session *session = (_MetisCommandLineInterface_Session *) cliSessionVoid;
    PARCEventBuffer *input = parcEventBuffer_GetQueueBufferInput(event);

    while (parcEventBuffer_GetLength(input) > 0) {
        size_t readLength = 0;
        char *cmdline = parcEventBuffer_ReadLine(input, &readLength);
        if (cmdline == NULL) {
            // we have run out of input, we're done
            parcEventBuffer_Destroy(&input);
            return;
        }

        // we have a whole command line
        bool success = _metisCliSession_ProcessCommand(session, cmdline);
        parcEventBuffer_FreeLine(input, &cmdline);

        if (!success) {
            // the session is dead
            parcEventBuffer_Destroy(&input);
            return;
        }

        _metisCliSession_DisplayPrompt(session);
    }
    parcEventBuffer_Destroy(&input);
}

static void
_metisCommandLineInterface_RemoveSession(MetisCommandLineInterface *cli, _MetisCommandLineInterface_Session *session)
{
    size_t length = parcArrayList_Size(cli->openSessions);
    for (size_t i = 0; i < length; i++) {
        _MetisCommandLineInterface_Session *x = parcArrayList_Get(cli->openSessions, i);
        if (x == session) {
            // removing from list will call the session destroyer
            parcArrayList_RemoveAndDestroyAtIndex(cli->openSessions, i);
            return;
        }
    }
    assertTrue(0, "Illegal state: did not find a session in openSessions %p", (void *) session);
}

static void
_metisCliSession_EventCallback(PARCEventQueue *event, PARCEventQueueEventType what, void *cliSessionVoid)
{
    _MetisCommandLineInterface_Session *session = (_MetisCommandLineInterface_Session *) cliSessionVoid;
    if (what & PARCEventQueueEventType_Error) {
        MetisCommandLineInterface *cli = session->parentCli;
        _metisCommandLineInterface_RemoveSession(cli, session);
    }
}

static void
_metisCliSession_DisplayMotd(_MetisCommandLineInterface_Session *session)
{
    parcEventQueue_Printf(session->streamBuffer, "Metis Forwarder CLI\n");
    parcEventQueue_Printf(session->streamBuffer, "Copyright (c) 2017 Cisco and/or its affiliates.\n\n");

    parcEventQueue_Flush(session->streamBuffer, PARCEventType_Write);
}

static void
_metisCliSession_DisplayPrompt(_MetisCommandLineInterface_Session *session)
{
    parcEventQueue_Printf(session->streamBuffer, "metis> ");
    parcEventQueue_Flush(session->streamBuffer, PARCEventType_Write);
}

/**
 * Process commands until there's not a whole line (upto CRLF)
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return false if the session died, true if its still going
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static bool
_metisCliSession_ProcessCommand(_MetisCommandLineInterface_Session *session, char *cmdline)
{
    // parse out the first word.  The NULL goes in after a space or tab.
    // "cmd" will be the string prior to the NULL, and "cmdline" will be what's after the NULL.
    char *cmd = strsep(&cmdline, " \t");

    // there's a secret command for unit testing
    if (strcasecmp(cmd, "~~") == 0) {
        parcEventQueue_Printf(session->streamBuffer, "success: %s\n", cmdline);
        return true;
    }

    for (int i = 0; _cliCommands[i].text != NULL; i++) {
        if (strcasecmp(_cliCommands[i].text, cmd) == 0) {
            _cliCommands[i].func(session, &_cliCommands[i], cmdline);
            if (_cliCommands[i].func == _cmd_Exit) {
                return false;
            }
            return true;
        }
    }

    // could not find command, print the help menu
    parcEventQueue_Printf(session->streamBuffer, "Unrecognized command: %s, displaying help menu\n", cmdline);
    _cmd_Help(session, NULL, NULL);
    return true;
}

static void
_cmd_Help(_MetisCommandLineInterface_Session *session, _MetisCommandLineInterface_Command *command, const char *params)
{
    for (int i = 0; _cliCommands[i].text != NULL; i++) {
        parcEventQueue_Printf(session->streamBuffer, "%-8s %s\n", _cliCommands[i].text, _cliCommands[i].helpDescription);
    }
}

static void
_cmd_Show(_MetisCommandLineInterface_Session *session, _MetisCommandLineInterface_Command *command, const char *params)
{
    parcEventQueue_Printf(session->streamBuffer, "not implemented\n");
}

static void
_cmd_Tunnel(_MetisCommandLineInterface_Session *session, _MetisCommandLineInterface_Command *command, const char *params)
{
    parcEventQueue_Printf(session->streamBuffer, "not implemented\n");
}

static void
_cmd_Exit(_MetisCommandLineInterface_Session *session, _MetisCommandLineInterface_Command *command, const char *params)
{
    parcEventQueue_Printf(session->streamBuffer, "Exiting session, goodby\n\n");
    parcEventQueue_Flush(session->streamBuffer, PARCEventType_Write);
    _metisCommandLineInterface_RemoveSession(session->parentCli, session);
}

static void
_cmd_Version(_MetisCommandLineInterface_Session *session, _MetisCommandLineInterface_Command *command, const char *params)
{
    PARCJSON *versionJson = metisConfiguration_GetVersion(metisForwarder_GetConfiguration(session->parentCli->metis));
    char *str = parcJSON_ToString(versionJson);
    parcEventQueue_Printf(session->streamBuffer, "%s", str);
    parcMemory_Deallocate((void **) &str);
    parcJSON_Release(&versionJson);
}
