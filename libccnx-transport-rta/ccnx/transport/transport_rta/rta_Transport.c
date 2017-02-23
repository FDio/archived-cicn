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
 * This is the API-thread's interface to the RTA framework.  It is thread-safe
 * and executes in the API's thread.
 *
 * The only data maintained here is a mapping from the SYSTEM parameters hash
 * to the stack_id.
 *
 * Communication with the Framework is done over a socket pair.
 */
#include <config.h>

#include <LongBow/runtime.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>

#include <parc/algol/parc_Memory.h>
//#include <parc/logging/parc_Log.h>
//#include <parc/logging/parc_LogReporterTextStdout.h>
#include <parc/concurrent/parc_RingBuffer_1x1.h>
#include <parc/concurrent/parc_Notifier.h>
#include <parc/algol/parc_Deque.h>
#include <parc/concurrent/parc_Synchronizer.h>

#include <ccnx/transport/transport_rta/rta_Transport.h>
#include <ccnx/transport/common/transport_private.h>
#include <ccnx/transport/transport_rta/core/rta_Framework.h>
#include <ccnx/transport/transport_rta/core/rta_ProtocolStack.h>
#include <ccnx/transport/transport_rta/commands/rta_Command.h>
#include <ccnx/transport/transport_rta/core/components.h>
#include <ccnx/transport/transport_rta/core/rta_ConnectionTable.h>

// These are some internal diagnostic counters used in the debugger
// for when things are going really bad.  They are incremented on each
// call to read or write.
unsigned rta_transport_reads = 0;
unsigned rta_transport_read_spin = 0;
unsigned rta_transport_writes = 0;

// ===================================================
// The external interface

const struct transport_operations rta_ops = {
    .Create      = (void * (*)(void))rtaTransport_Create,
    .Open        = (int (*)(void *,                       CCNxTransportConfig *))rtaTransport_Open,
    .Send        = (int (*)(void *,                       int,                     CCNxMetaMessage *,   const struct timeval *restrict timeout))rtaTransport_Send,
    .Recv        = (TransportIOStatus (*)(void *,         int,                     CCNxMetaMessage **,  const struct timeval *restrict timeout))rtaTransport_Recv,
    .Close       = (int (*)(void *,                       int))rtaTransport_Close,
    .Destroy     = (int (*)(void **))rtaTransport_Destroy,
    .PassCommand = (int (*)(void *,                       void *))rtaTransport_PassCommand
};

/**
 * @typedef _StackEntry
 * @abstract Tracks the JSON descriptions of protocol stacks
 * @constant hash The hash of the JSON description
 * @constant stack_id the id of the stack associated with that hash
 * @constant list The linked-list member
 * @discussion <#Discussion#>
 */
typedef struct json_hash_table {
    PARCHashCode hash;
    int stack_id;
} _StackEntry;

typedef struct socket_pair {
    int up;
    int down;
} _RTASocketPair;

struct rta_transport {
    RtaFramework  *framework;    /**< The RTA Framework holding the transport */

    PARCRingBuffer1x1 *commandRingBuffer; /**< Written from Transport down to Framework */

    PARCNotifier *commandNotifier; /**< Shared with the Framework to indicates writes to the ring buffer */

    unsigned int nextStackId;

    PARCDeque *list;
};

static _StackEntry *
_rtaTransport_GetStack(const RTATransport *transport, PARCHashCode hash)
{
    _StackEntry *result = NULL;

    PARCIterator *iterator = parcDeque_Iterator(transport->list);
    while (parcIterator_HasNext(iterator)) {
        _StackEntry *entry = parcIterator_Next(iterator);
        if (entry->hash == hash) {
            result = entry;
            break;
        }
    }
    parcIterator_Release(&iterator);

    return result;
}

static _StackEntry *
_rtaTransport_AddStack(RTATransport *transport, CCNxStackConfig *stackConfig)
{
    PARCHashCode hash = ccnxStackConfig_HashCode(stackConfig);

    _StackEntry *entry = parcMemory_AllocateAndClear(sizeof(_StackEntry));
    assertNotNull(entry, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(_StackEntry));
    entry->hash = hash;
    entry->stack_id = transport->nextStackId++;

    parcDeque_Append(transport->list, entry);

    return entry;
}

static void
_rtaTransport_CommandBufferEntryDestroyer(void **entryPtr)
{
}

static bool
_rtaTransport_SendCommandToFramework(RTATransport *transport, const RtaCommand *command)
{
    bool success = rtaCommand_Write(command, transport->commandRingBuffer);
    if (success) {
        parcNotifier_Notify(transport->commandNotifier);
        return true;
    }
    return false;
}

RTATransport *
rtaTransport_Create(void)
{
    RTATransport *transport = parcMemory_AllocateAndClear(sizeof(RTATransport));

    if (transport != NULL) {
        transport->nextStackId = 1;

        transport->commandRingBuffer = parcRingBuffer1x1_Create(128, _rtaTransport_CommandBufferEntryDestroyer);
        transport->commandNotifier = parcNotifier_Create();

        transport->framework = rtaFramework_Create(transport->commandRingBuffer, transport->commandNotifier);
        assertNotNull(transport->framework, "rtaFramework_Create returned null");

        rtaFramework_Start(transport->framework);
        transport->list = parcDeque_Create();
    }

    return transport;
}

int
rtaTransport_Destroy(RTATransport **ctxPtr)
{
    assertNotNull(ctxPtr, "called with null context pointer");
    RTATransport *transport = *ctxPtr;

    // %%%%% LOCK (notice this lock never gets unlocked, it just gets deleted)
    parcDeque_Lock(transport->list);

    // This blocks until shutdown (state FRAMEWORK_SHUTDOWN)
    rtaFramework_Shutdown(transport->framework);

    // This will close and drain all the API fds
    rtaFramework_Destroy(&transport->framework);

    parcNotifier_Release(&transport->commandNotifier);
    parcRingBuffer1x1_Release(&transport->commandRingBuffer);

    // Destroy the state we have stored locally to map JSON protocol stack descriptions
    // to stack_id identifiers.

    for (size_t index = 0; index < parcDeque_Size(transport->list); index++) {
        _StackEntry *entry = parcDeque_GetAtIndex(transport->list, index);
        parcMemory_Deallocate((void **) &entry);
    }

    parcDeque_Release(&transport->list);

    parcMemory_Deallocate((void **) ctxPtr);

//    printf("rta_transport writes=%9u reads=%9u spins=%9u\n", rta_transport_writes, rta_transport_reads, rta_transport_read_spin);
    return 0;
}

static _RTASocketPair
_rtaTransport_CreateSocketPair(const RTATransport *transport, int bufferSize)
{
    int fds[2];

    bool success = (socketpair(PF_LOCAL, SOCK_STREAM, 0, fds) == 0);
    assertTrue(success, "socketpair(PF_LOCAL, SOCK_STREAM, ...) failed.");

    _RTASocketPair result = { .up = fds[0], .down = fds[1] };

    // Set buffer size
    int sendbuff = bufferSize;

    success = (setsockopt(result.up, SOL_SOCKET, SO_RCVBUF, &sendbuff, sizeof(sendbuff)) == 0);
    assertTrue(success, "Expected success for setsockopt SO_RCVBUF");

    success = (setsockopt(result.down, SOL_SOCKET, SO_RCVBUF, &sendbuff, sizeof(sendbuff)) == 0);
    assertTrue(success, "Expected success for setsockopt SO_RCVBUF");

    return result;
}

/**
 * Returns the protocol stack entry from our table
 *
 * Determine if we already have a protocol stack with the same structure as the user asks for.
 * If so, return that entry, otherwise return NULL
 *
 * @param [in] transport The RTA transport
 * @param [in] transportConfig the configuration the user is asking for
 *
 * @return non-NULL The existing protocol stack holder
 * @return NULL Configuration does not exist
 */
static _StackEntry *
_rtaTransport_GetProtocolStackEntry(RTATransport *transport, CCNxTransportConfig *transportConfig)
{
    PARCHashCode hash = ccnxStackConfig_HashCode(ccnxTransportConfig_GetStackConfig(transportConfig));

    _StackEntry *stack = _rtaTransport_GetStack(transport, hash);
    return stack;
}

/**
 * Add a protocol stack
 *
 * Adds an entry to our local table of Config -> stack_id mapping and sends a
 * command over the command socket to create the protocol stack.
 *
 * @param [in] transport The RTA transport
 * @param [in] transportConfig the user specified configuration
 *
 * @return non-NULL The holder of the protocol stack mapping
 * @return NULL An error
 */
static _StackEntry *
_rtaTransport_AddProtocolStackEntry(RTATransport *transport, const CCNxTransportConfig *transportConfig)
{
    CCNxStackConfig *stackConfig = ccnxTransportConfig_GetStackConfig(transportConfig);

    _StackEntry *stack = _rtaTransport_AddStack(transport, stackConfig);

    RtaCommandCreateProtocolStack *createStack = rtaCommandCreateProtocolStack_Create(stack->stack_id, stackConfig);

    // request for a new protocol stack, create it

    // now actually create the protocol stack by writing a command over the thread boundary
    // using the Command socket.
    RtaCommand *command = rtaCommand_CreateCreateProtocolStack(createStack);
    _rtaTransport_SendCommandToFramework(transport, command);

    rtaCommand_Release(&command);
    rtaCommandCreateProtocolStack_Release(&createStack);

    return stack;
}

/**
 * Create a new connection
 *
 * We have resolved that a matching protocol stack exists, and is represented by
 * protocolStackHashEntry.  We now want to send a command over the command socket to
 * create a connection in that stack.
 *
 * @param [in] transport The RTA transport
 * @param [in] transportConfig The user requested configuration
 * @param [in] protocolStackHashEntry The protocol stack holder
 * @param [in] pair A _RTASocketPair representing the queue of data between the API and the transport stack.
 */
static void
_rtaTransport_CreateConnection(RTATransport *transport, CCNxTransportConfig *transportConfig, _StackEntry *stack, _RTASocketPair pair)
{
    RtaCommandOpenConnection *openConnection =
        rtaCommandOpenConnection_Create(stack->stack_id,
                                        pair.up,
                                        pair.down,
                                        ccnxConnectionConfig_GetJson(ccnxTransportConfig_GetConnectionConfig(transportConfig)));

    RtaCommand *command = rtaCommand_CreateOpenConnection(openConnection);
    _rtaTransport_SendCommandToFramework(transport, command);

    rtaCommand_Release(&command);
    rtaCommandOpenConnection_Release(&openConnection);
}

int
rtaTransport_Open(RTATransport *transport, CCNxTransportConfig *transportConfig)
{
    ccnxTransportConfig_OptionalAssertValid(transportConfig);

    assertNotNull(transport, "Parameter transport must be a valid RTATransport");

    _RTASocketPair pair = _rtaTransport_CreateSocketPair(transport, sizeof(void *) * 128);

    parcDeque_Lock(transport->list);
    {
        _StackEntry *stack = _rtaTransport_GetProtocolStackEntry(transport, transportConfig);
        if (stack == NULL) {
            stack = _rtaTransport_AddProtocolStackEntry(transport, transportConfig);
        }
        assertNotNull(stack, "Got NULL hash entry from _rtaTransport_AddProtocolStackEntry");

        _rtaTransport_CreateConnection(transport, transportConfig, stack, pair);
    }
    parcDeque_Unlock(transport->list);

    return pair.up;
}

/**
 * timeout is either NULL or a pointer to an unsigned integer containing the number of microseconds to wait for input.
 *
 * @return <0  An error occured
 * @return 0   A timeout occurred waiting for the filedescriptor to have some output space available.
 * @return >0  The filedescriptor has some output space available.
 */
static int
_rtaTransport_SendSelect(const int fd, const uint64_t *microSeconds)
{
    struct timeval timeval;
    fd_set writeSet;

    FD_ZERO(&writeSet); // clear the set
    FD_SET(fd, &writeSet); // add our file descriptor to the set

    struct timeval *timeout = NULL;

    if (microSeconds != NULL) {
        timeval.tv_sec = (int) (*microSeconds / 1000000);
        timeval.tv_usec = (int) (*microSeconds % 1000000);
        timeout = &timeval;
    }

    int selectResult = select(fd + 1, NULL, &writeSet, NULL, timeout);

    return selectResult;
}

bool
rtaTransport_Send(RTATransport *transport, int queueId, const CCNxMetaMessage *message, const uint64_t *microSeconds)
{
    // Acquire a reference to the incoming CCNxMetaMessage so if the caller releases it immediately,
    // a reference still exists for the transport. This reference is released once the
    // message is processed lower in the stack.
    CCNxMetaMessage *metaMessage = ccnxMetaMessage_Acquire(message);

    rta_transport_writes++;

    int selectResult = _rtaTransport_SendSelect(queueId, microSeconds);
    if (selectResult < 0) {
        // We couldn't send it. Release our reference and return signaling failure.
        ccnxMetaMessage_Release(&metaMessage);
        return false;
    } else if (selectResult == 0) {
        errno = EWOULDBLOCK;
        ccnxMetaMessage_Release(&metaMessage);
        return false;
    } else if (selectResult > 0) {
        ssize_t count = write(queueId, &metaMessage, sizeof(&metaMessage));
        if (count == sizeof(&metaMessage)) {
            return true;
        }
    }

    // We couldn't send it. Release our reference and return signaling failure.
    ccnxMetaMessage_Release(&metaMessage);

    return false;
}

//#if 1
/**
 * @return -1  An error occured
 * @return 0  A timeout occurred waiting for the filedescriptor to have some input available.
 * @return >0  The filedescriptor has some input ready.
 */
static int
_rtaTransport_ReceiveSelect(const int fd, const uint64_t *microSeconds)
{
    fd_set readSet;

    FD_ZERO(&readSet); // clear the set
    FD_SET(fd, &readSet); // add our file descriptor to the set

    struct timeval *timeout = NULL;
    struct timeval timeval;

    if (microSeconds != NULL) {
        timeval.tv_sec = (int) (*microSeconds / 1000000);
        timeval.tv_usec = (int) (*microSeconds % 1000000);
        timeout = &timeval;
    }
    int selectResult = select(fd + 1, &readSet, NULL, NULL, (struct timeval *) timeout);

    return selectResult;
}

TransportIOStatus
rtaTransport_Recv(RTATransport *transport, const int queueId, CCNxMetaMessage **msgPtr, const uint64_t *microSeconds)
{
    // The effect here is to transfer the reference to the CCNxMetaMessage to the application-side thread.
    // Thus, no acquire or release here as the caller is responsible for releasing the CCNxMetaMessage

    int selectResult = _rtaTransport_ReceiveSelect(queueId, microSeconds);

    if (selectResult == -1) {
        // errno should have been set by the select(2) system call.
        return TransportIOStatus_Error;
    } else if (selectResult == 0) {
        //        errno = EWOULDBLOCK;
        errno = ENOMSG;
        return TransportIOStatus_Timeout;
    }

    size_t remaining = sizeof(&*msgPtr);
    uint8_t *bytes = (uint8_t *) msgPtr;

    do {
        ssize_t nread = read(queueId, &bytes[sizeof(&*msgPtr) - remaining], remaining);
        if (nread == -1 && errno != EINTR) {
            return TransportIOStatus_Error;
        }
        if (nread == 0) {
            rta_transport_read_spin++;
        }
        remaining -= nread;
    }  while (remaining > 0);

    rta_transport_reads++;

    errno = 0;
    return TransportIOStatus_Success;
}
//#else
///**
// * @return -1  An error occured
// * @return 0  A timeout occurred waiting for the filedescriptor to have some input available.
// * @return >0  The filedescriptor has some input ready.
// */
//static int
//_rtaTransport_Select(const int fd, const struct timeval *restrict timeout)
//{
//    fd_set readSet;
//
//    FD_ZERO(&readSet); // clear the set
//    FD_SET(fd, &readSet); // add our file descriptor to the set
//
//    int selectResult = select(fd + 1, &readSet, NULL, NULL, (struct timeval *) timeout);
//
//    return selectResult;
//}
//
//TransportIOStatus
//rtaTransport_Recv(RTATransport *transport, const int queueId, CCNxMetaMessage **msgPtr, const struct timeval *restrict timeout)
//{
//    // The effect here is to transfer the reference to the CCNxMetaMessage to the application-side thread.
//    // Thus, no acquire or release here as the caller is responsible for releasing the CCNxMetaMessage
//
//    int selectResult = _rtaTransport_Select(queueId, timeout);
//
//    if (selectResult == -1) {
//        // errno should have been set by the select(2) system call.
//        return TransportIOStatus_Error;
//    } else if (selectResult == 0) {
////        errno = EWOULDBLOCK;
//        errno = ENOMSG;
//        return TransportIOStatus_Timeout;
//    }
//
//    size_t remaining = sizeof(&*msgPtr);
//    uint8_t *bytes = (uint8_t *) msgPtr;
//
//    do {
//        ssize_t nread = read(queueId, &bytes[sizeof(&*msgPtr) - remaining], remaining);
//        if (nread == -1 && errno != EINTR) {
//            return TransportIOStatus_Error;
//        }
//        if (nread == 0) {
//            rta_transport_read_spin++;
//        }
//        remaining -= nread;
//    }  while (remaining > 0);
//
//    rta_transport_reads++;
//
//    errno = 0;
//    return TransportIOStatus_Success;
//}
//#endif

int
rtaTransport_Close(RTATransport *transport, int api_fd)
{
    RtaCommandCloseConnection *commandClose = rtaCommandCloseConnection_Create(api_fd);
    RtaCommand *command = rtaCommand_CreateCloseConnection(commandClose);
    rtaCommandCloseConnection_Release(&commandClose);

    _rtaTransport_SendCommandToFramework(transport, command);

    rtaCommand_Release(&command);

    return 0;
}

int
rtaTransport_PassCommand(RTATransport *transport, const RtaCommand *rtacommand)
{
    _rtaTransport_SendCommandToFramework(transport, rtacommand);

    return 0;
}
