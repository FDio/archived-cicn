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

/**
 * Implements the single wrapper for commands sent over the command channel
 *
 */

#include <config.h>

#include <LongBow/runtime.h>

#include <stdio.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <ccnx/transport/transport_rta/commands/rta_Command.h>

/*
 * Internal definition of command types
 */
typedef enum {
    RtaCommandType_Unknown = 0,
    RtaCommandType_CreateProtocolStack,
    RtaCommandType_OpenConnection,
    RtaCommandType_CloseConnection,
    RtaCommandType_DestroyProtocolStack,
    RtaCommandType_ShutdownFramework,
    RtaCommandType_TransmitStatistics,
    RtaCommandType_Last
} _RtaCommandType;

struct rta_command {
    _RtaCommandType type;

    union {
        RtaCommandCloseConnection *closeConnection;
        RtaCommandOpenConnection *openConnection;
        RtaCommandCreateProtocolStack *createStack;
        RtaCommandDestroyProtocolStack *destroyStack;
        RtaCommandTransmitStatistics *transmitStats;

        // shutdown framework has no value it will be NULL
        // Statistics has no value
        void *noValue;
    } value;
};

static struct commandtype_to_string {
    _RtaCommandType type;
    const char *string;
} _RtaCommandTypeToString[] = {
    { .type = RtaCommandType_Unknown,              .string = "Unknown"              },
    { .type = RtaCommandType_CreateProtocolStack,  .string = "CreateProtocolStack"  },
    { .type = RtaCommandType_OpenConnection,       .string = "OpenConnection"       },
    { .type = RtaCommandType_CloseConnection,      .string = "CloseConnection"      },
    { .type = RtaCommandType_DestroyProtocolStack, .string = "DestroyProtocolStack" },
    { .type = RtaCommandType_ShutdownFramework,    .string = "ShutdownFramework"    },
    { .type = RtaCommandType_TransmitStatistics,   .string = "TransmitStatistics"   },
    { .type = RtaCommandType_Last,                 .string = NULL                   },
};

// ===============================
// Internal API

static void
_rtaCommand_Destroy(RtaCommand **commandPtr)
{
    RtaCommand *command = *commandPtr;
    switch (command->type) {
        case RtaCommandType_ShutdownFramework:
            // no inner-release needed
            break;

        case RtaCommandType_CreateProtocolStack:
            rtaCommandCreateProtocolStack_Release(&command->value.createStack);
            break;

        case RtaCommandType_OpenConnection:
            rtaCommandOpenConnection_Release(&command->value.openConnection);
            break;

        case RtaCommandType_CloseConnection:
            rtaCommandCloseConnection_Release(&command->value.closeConnection);
            break;

        case RtaCommandType_DestroyProtocolStack:
            rtaCommandDestroyProtocolStack_Release(&command->value.destroyStack);
            break;

        case RtaCommandType_TransmitStatistics:
            rtaCommandTransmitStatistics_Release(&command->value.transmitStats);
            break;

        default:
            trapIllegalValue(command->type, "Illegal command type %d", command->type);
            break;
    }
}

#ifdef Transport_DISABLE_VALIDATION
#  define _rtaCommand_OptionalAssertValid(_instance_)
#else
#  define _rtaCommand_OptionalAssertValid(_instance_) _rtaCommand_AssertValid(_instance_)
#endif

static void
_rtaCommand_AssertValid(const RtaCommand *command)
{
    assertNotNull(command, "RtaCommand must be non-null");
    assertTrue(RtaCommandType_Unknown < command->type && command->type < RtaCommandType_Last,
               "Invalid RtaCommand type, must be %d < type %d < %d",
               RtaCommandType_Unknown,
               command->type,
               RtaCommandType_Last);

    switch (command->type) {
        case RtaCommandType_ShutdownFramework:
            assertNull(command->value.noValue, "RtaCommand value must be null for ShutdownFramework or Statistics");
            break;

        case RtaCommandType_CreateProtocolStack:
            assertNotNull(command->value.createStack, "RtaCommand createStack member must be non-null");
            break;

        case RtaCommandType_OpenConnection:
            assertNotNull(command->value.openConnection, "RtaCommand openConnection member must be non-null");
            break;

        case RtaCommandType_CloseConnection:
            assertNotNull(command->value.closeConnection, "RtaCommand closeConnection member must be non-null");
            break;

        case RtaCommandType_DestroyProtocolStack:
            assertNotNull(command->value.destroyStack, "RtaCommand destroyStack member must be non-null");
            break;

        case RtaCommandType_TransmitStatistics:
            assertNotNull(command->value.transmitStats, "RtaCommand transmitStats member must be non-null");
            break;

        default:
            trapIllegalValue(command->type, "Illegal command type %d", command->type);
            break;
    }
}

parcObject_ExtendPARCObject(RtaCommand, _rtaCommand_Destroy, NULL, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementAcquire(rtaCommand, RtaCommand);

parcObject_ImplementRelease(rtaCommand, RtaCommand);

static RtaCommand *
_rtaCommand_Allocate(_RtaCommandType type)
{
    RtaCommand *command = parcObject_CreateInstance(RtaCommand);
    command->type = type;
    return command;
}

static const char *
_rtaCommand_TypeToString(const RtaCommand *command)
{
    for (int i = 0; _RtaCommandTypeToString[i].string != NULL; i++) {
        if (_RtaCommandTypeToString[i].type == command->type) {
            return _RtaCommandTypeToString[i].string;
        }
    }
    trapUnexpectedState("Command is not a valid type: %d", command->type);
}

// ===============================
// Public API

void
rtaCommand_Display(const RtaCommand *command, int indentation)
{
    _rtaCommand_OptionalAssertValid(command);

    parcDisplayIndented_PrintLine(indentation, "RtaCommand type %s (%d) value pointer %p\n",
                                  _rtaCommand_TypeToString(command), command->type, command->value);
}

/*
 * Gets a reference to itself and puts it in the ring buffer
 */
bool
rtaCommand_Write(const RtaCommand *command, PARCRingBuffer1x1 *commandRingBuffer)
{
    _rtaCommand_OptionalAssertValid(command);

    RtaCommand *reference = rtaCommand_Acquire(command);

    bool addedToRingBuffer = parcRingBuffer1x1_Put(commandRingBuffer, reference);

    if (!addedToRingBuffer) {
        // it was not stored in the ring, so we need to be responsible and release it
        rtaCommand_Release(&reference);
    }

    return addedToRingBuffer;
}

RtaCommand *
rtaCommand_Read(PARCRingBuffer1x1 *commandRingBuffer)
{
    RtaCommand *referenceFromRing = NULL;

    bool fetchedReference = parcRingBuffer1x1_Get(commandRingBuffer, (void **) &referenceFromRing);
    if (fetchedReference) {
        return referenceFromRing;
    }
    return NULL;
}

// ======================
// CLOSE CONNECTION

bool
rtaCommand_IsCloseConnection(const RtaCommand *command)
{
    _rtaCommand_OptionalAssertValid(command);
    return (command->type == RtaCommandType_CloseConnection);
}

RtaCommand *
rtaCommand_CreateCloseConnection(const RtaCommandCloseConnection *close)
{
    RtaCommand *command = _rtaCommand_Allocate(RtaCommandType_CloseConnection);
    command->value.closeConnection = rtaCommandCloseConnection_Acquire(close);
    return command;
}

const RtaCommandCloseConnection *
rtaCommand_GetCloseConnection(const RtaCommand *command)
{
    assertTrue(rtaCommand_IsCloseConnection(command), "Command is not CloseConnection");
    return command->value.closeConnection;
}

// ======================
// OPEN CONNECTION

bool
rtaCommand_IsOpenConnection(const RtaCommand *command)
{
    _rtaCommand_OptionalAssertValid(command);
    return (command->type == RtaCommandType_OpenConnection);
}

RtaCommand *
rtaCommand_CreateOpenConnection(const RtaCommandOpenConnection *open)
{
    RtaCommand *command = _rtaCommand_Allocate(RtaCommandType_OpenConnection);
    command->value.openConnection = rtaCommandOpenConnection_Acquire(open);
    return command;
}

const RtaCommandOpenConnection *
rtaCommand_GetOpenConnection(const RtaCommand *command)
{
    assertTrue(rtaCommand_IsOpenConnection(command), "Command is not OpenConnection");
    return command->value.openConnection;
}

// ======================
// CREATE STACK

bool
rtaCommand_IsCreateProtocolStack(const RtaCommand *command)
{
    _rtaCommand_OptionalAssertValid(command);
    return (command->type == RtaCommandType_CreateProtocolStack);
}

RtaCommand *
rtaCommand_CreateCreateProtocolStack(const RtaCommandCreateProtocolStack *createStack)
{
    RtaCommand *command = _rtaCommand_Allocate(RtaCommandType_CreateProtocolStack);
    command->value.createStack = rtaCommandCreateProtocolStack_Acquire(createStack);
    return command;
}

const RtaCommandCreateProtocolStack *
rtaCommand_GetCreateProtocolStack(const RtaCommand *command)
{
    assertTrue(rtaCommand_IsCreateProtocolStack(command), "Command is not CreateProtocolStack");
    return command->value.createStack;
}

bool
rtaCommand_IsDestroyProtocolStack(const RtaCommand *command)
{
    _rtaCommand_OptionalAssertValid(command);
    return (command->type == RtaCommandType_DestroyProtocolStack);
}

RtaCommand *
rtaCommand_CreateDestroyProtocolStack(const RtaCommandDestroyProtocolStack *destroyStack)
{
    RtaCommand *command = _rtaCommand_Allocate(RtaCommandType_DestroyProtocolStack);
    command->value.destroyStack = rtaCommandDestroyProtocolStack_Acquire(destroyStack);
    return command;
}

const RtaCommandDestroyProtocolStack *
rtaCommand_GetDestroyProtocolStack(const RtaCommand *command)
{
    assertTrue(rtaCommand_IsDestroyProtocolStack(command), "Command is not DestroyProtocolStack");
    return command->value.destroyStack;
}

bool
rtaCommand_IsShutdownFramework(const RtaCommand *command)
{
    _rtaCommand_OptionalAssertValid(command);
    return (command->type == RtaCommandType_ShutdownFramework);
}

RtaCommand *
rtaCommand_CreateShutdownFramework(void)
{
    RtaCommand *command = _rtaCommand_Allocate(RtaCommandType_ShutdownFramework);
    command->value.noValue = NULL;
    return command;
}

// no getter

bool
rtaCommand_IsTransmitStatistics(const RtaCommand *command)
{
    _rtaCommand_OptionalAssertValid(command);
    return (command->type == RtaCommandType_TransmitStatistics);
}

RtaCommand *
rtaCommand_CreateTransmitStatistics(const RtaCommandTransmitStatistics *transmitStats)
{
    RtaCommand *command = _rtaCommand_Allocate(RtaCommandType_TransmitStatistics);
    command->value.transmitStats = rtaCommandTransmitStatistics_Acquire(transmitStats);
    return command;
}

const RtaCommandTransmitStatistics *
rtaCommand_GetTransmitStatistics(const RtaCommand *command)
{
    assertTrue(rtaCommand_IsTransmitStatistics(command), "Command is not TransmitStatistics");
    return command->value.transmitStats;
}
