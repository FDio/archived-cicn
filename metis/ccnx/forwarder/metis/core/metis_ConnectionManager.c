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
 * The Connection Manager sets itself up as a listener to the Messenger so it can take
 * action based on system events.
 *
 * The Connection Manager queues and then processes in a later time slice the messages.
 *
 */

#include <config.h>
#include <stdio.h>
#include <ccnx/forwarder/metis/core/metis_ConnectionManager.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/messenger/metis_Messenger.h>
#include <ccnx/forwarder/metis/messenger/metis_MessengerRecipient.h>
#include <ccnx/forwarder/metis/messenger/metis_MissiveDeque.h>

#include <parc/algol/parc_Memory.h>

#include <LongBow/runtime.h>

struct metis_connection_manager {
    MetisForwarder *metis;
    MetisLogger *logger;

    MetisMessengerRecipient *messengerRecipient;

    // we queue missives as they come in to process in our own
    // event timeslice
    MetisMissiveDeque *missiveQueue;

    // for deferred queue processing
    PARCEventTimer *timerEvent;
};

/**
 * Receives missives from the messenger, queues them, and schedules our execution
 *
 * We defer processing of missives to a later time slice
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void metisConnectionManager_MessengerCallback(MetisMessengerRecipient *recipient, MetisMissive *missive);

/**
 * Event callback
 *
 * This is our main run loop to process our queue of messages.  It is scheduled
 * in {@link metisConnectionManager_MessengerCallback} when the queue becomes non-empty.
 *
 * When we are called here, we have exclusive use of the system, so we will not create any message loops
 *
 * @param [in] fd unused, required for compliance with function prototype
 * @param [in] which_event unused, required for compliance with function prototype
 * @param [in] connManagerVoidPtr A void* to MetisConnectionManager
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void metisConnectionManager_ProcessQueue(int fd, PARCEventType which_event, void *connManagerVoidPtr);

/**
 * Process a missive for a connection DOWN
 *
 * We've dequeued a missive and are now ready to take action on it
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void metisConnectionManager_ProcessDownMissive(MetisConnectionManager *connManager, const MetisMissive *missive);

/**
 * Process a missive for a connection UP
 *
 * We've dequeued a missive and are now ready to take action on it
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void metisConnectionManager_ProcessUpMissive(MetisConnectionManager *connManager, const MetisMissive *missive);

static void metisConnectionManager_ProcessCreateMissive(MetisConnectionManager *connManager, const MetisMissive *missive);
static void metisConnectionManager_ProcessClosedMissive(MetisConnectionManager *connManager, const MetisMissive *missive);
static void metisConnectionManager_ProcessDestroyedMissive(MetisConnectionManager *connManager, const MetisMissive *missive);

/**
 * Send a notification up to local applications about connection state changes.
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static void metisConnectionManager_NotifyApplications(MetisConnectionManager *connManager, const MetisMissive *missive);

// ========================================================
// Public API

MetisConnectionManager *
metisConnectionManager_Create(MetisForwarder *metis)
{
    MetisConnectionManager *connManager = parcMemory_AllocateAndClear(sizeof(MetisConnectionManager));
    assertNotNull(connManager, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisConnectionManager));
    connManager->metis = metis;
    connManager->missiveQueue = metisMissiveDeque_Create();
    connManager->logger = metisLogger_Acquire(metisForwarder_GetLogger(metis));

    MetisMessenger *messenger = metisForwarder_GetMessenger(connManager->metis);

    // creates the timer, but does not start it
    PARCEventScheduler *base = metisDispatcher_GetEventScheduler(metisForwarder_GetDispatcher(metis));
    connManager->timerEvent = parcEventTimer_Create(base, 0, metisConnectionManager_ProcessQueue, connManager);

    connManager->messengerRecipient = metisMessengerRecipient_Create(connManager, metisConnectionManager_MessengerCallback);
    metisMessenger_Register(messenger, connManager->messengerRecipient);
    return connManager;
}

void
metisConnectionManager_Destroy(MetisConnectionManager **managerPtr)
{
    assertNotNull(managerPtr, "Double pointer must be non-null");
    assertNotNull(*managerPtr, "Double pointer must dereference to non-null");

    MetisConnectionManager *connManager = *managerPtr;

    MetisMessenger *messenger = metisForwarder_GetMessenger(connManager->metis);
    parcEventTimer_Destroy(&(connManager->timerEvent));
    metisMessenger_Unregister(messenger, connManager->messengerRecipient);
    metisMessengerRecipient_Destroy(&connManager->messengerRecipient);
    metisMissiveDeque_Release(&connManager->missiveQueue);
    metisLogger_Release(&connManager->logger);

    parcMemory_Deallocate((void **) &connManager);
    *managerPtr = NULL;
}

// ========================================================
// Internal Functions

static void
metisConnectionManager_MessengerCallback(MetisMessengerRecipient *recipient, MetisMissive *missive)
{
    MetisConnectionManager *connManager = metisMessengerRecipient_GetRecipientContext(recipient);

    // we do not release our reference count, we store it until later
    // We are called with our own reference, so we do not need to acquire the missive here.
    metisMissiveDeque_Append(connManager->missiveQueue, missive);

    if (metisMissiveDeque_Size(connManager->missiveQueue) == 1) {
        // When it becomes non-empty, schedule {@link metisConnectionManager_ProcessQueue}
        struct timeval immediateTimeout = { 0, 0 };
        parcEventTimer_Start(connManager->timerEvent, &immediateTimeout);
    }
}

static void
metisConnectionManager_ProcessQueue(int fd, PARCEventType which_event, void *connManagerVoidPtr)
{
    MetisConnectionManager *connManager = (MetisConnectionManager *) connManagerVoidPtr;

    MetisMissive *missive;
    while ((missive = metisMissiveDeque_RemoveFirst(connManager->missiveQueue)) != NULL) {
        switch (metisMissive_GetType(missive)) {
            case MetisMissiveType_ConnectionCreate:
                metisConnectionManager_ProcessCreateMissive(connManager, missive);
                break;
            case MetisMissiveType_ConnectionUp:
                metisConnectionManager_ProcessUpMissive(connManager, missive);
                break;
            case MetisMissiveType_ConnectionDown:
                metisConnectionManager_ProcessDownMissive(connManager, missive);
                break;
            case MetisMissiveType_ConnectionClosed:
                metisConnectionManager_ProcessClosedMissive(connManager, missive);
                break;
            case MetisMissiveType_ConnectionDestroyed:
                metisConnectionManager_ProcessDestroyedMissive(connManager, missive);
                break;
            default:
                trapUnexpectedState("Missive %p of unknown type: %d", (void *) missive, metisMissive_GetType(missive));
        }
        metisMissive_Release(&missive);
    }
}

static void
metisConnectionManager_ProcessUpMissive(MetisConnectionManager *connManager, const MetisMissive *missive)
{
    metisLogger_Log(connManager->logger, MetisLoggerFacility_Core, PARCLogLevel_Debug, __func__,
                    "Processing UP message for connid %u",
                    metisMissive_GetConnectionId(missive));

    metisConnectionManager_NotifyApplications(connManager, missive);
}

static void
metisConnectionManager_ProcessDownMissive(MetisConnectionManager *connManager, const MetisMissive *missive)
{
    metisLogger_Log(connManager->logger, MetisLoggerFacility_Core, PARCLogLevel_Debug, __func__,
                    "Processing DOWN message for connid %u",
                    metisMissive_GetConnectionId(missive));

    metisConnectionManager_NotifyApplications(connManager, missive);
}

static void
metisConnectionManager_ProcessCreateMissive(MetisConnectionManager *connManager, const MetisMissive *missive)
{
    metisLogger_Log(connManager->logger, MetisLoggerFacility_Core, PARCLogLevel_Debug, __func__,
                    "Processing CREATE message for connid %u",
                    metisMissive_GetConnectionId(missive));

    metisConnectionManager_NotifyApplications(connManager, missive);
}

static void
metisConnectionManager_ProcessClosedMissive(MetisConnectionManager *connManager, const MetisMissive *missive)
{
    metisLogger_Log(connManager->logger, MetisLoggerFacility_Core, PARCLogLevel_Debug, __func__,
                    "Processing CLOSED message for connid %u",
                    metisMissive_GetConnectionId(missive));

    MetisConnectionTable *table = metisForwarder_GetConnectionTable(connManager->metis);
    const MetisConnection *conn = metisConnectionTable_FindById(table, metisMissive_GetConnectionId(missive));

    if (conn) {
        // this will destroy the connection if its the last reference count
        metisConnectionTable_Remove(table, conn);

        // remove from FIB
        metisForwarder_RemoveConnectionIdFromRoutes(connManager->metis, metisMissive_GetConnectionId(missive));

        // finally tell apps
        metisConnectionManager_NotifyApplications(connManager, missive);
    }
}

static void
metisConnectionManager_ProcessDestroyedMissive(MetisConnectionManager *connManager, const MetisMissive *missive)
{
    metisLogger_Log(connManager->logger, MetisLoggerFacility_Core, PARCLogLevel_Debug, __func__,
                    "Processing DESTROYED message for connid %u",
                    metisMissive_GetConnectionId(missive));

    metisConnectionManager_NotifyApplications(connManager, missive);
}


static void
metisConnectionManager_NotifyApplications(MetisConnectionManager *connManager, const MetisMissive *missive)
{
    //TODO
}
