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
 * Event based router based on TLVs
 *
 * This module is the glue around the event scheduler.
 * Its the packet i/o module.
 *
 * Packet processing is done in metis_Dispatcher.c, which is the actual wrapper around the event scheduler
 *
 * USAGE:
 *
 *  MetisForwarder *forwarder = metisForwarder_Create(NULL);
 *
 *  // do one of these
 *  metisForwarder_SetupAllListeners(forwarder);
 *  or
 *  metisForwarder_SetupFromConfigFile(forwarder, "metis.cfg");
 *
 *  // now run the event loop via the dispatcher
 *  MetisDispatcher *dispatcher = metisForwarder_GetDispatcher();
 *
 *  // you can call any of the Run method sequentially.
 *  // chose one of
 *  metisDispatcher_Run(dispatcher);
 *  metisDispatcher_RunCount(dispatcher, 100);
 *  metisDispatcher_RunDuration(dispatcher, &((struct timeval) {30, 0}));
 *
 *  metisForwarder_Destroy(&forwarder);
 */

#include <config.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <string.h>
#include <errno.h>

#include <arpa/inet.h>
#include <sys/socket.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/core/metis_ConnectionManager.h>
#include <ccnx/forwarder/metis/core/metis_ConnectionTable.h>
#include <ccnx/forwarder/metis/core/metis_Dispatcher.h>
#include <ccnx/forwarder/metis/config/metis_Configuration.h>
#include <ccnx/forwarder/metis/config/metis_ConfigurationFile.h>
#include <ccnx/forwarder/metis/config/metis_ConfigurationListeners.h>
#include <ccnx/forwarder/metis/config/metis_CommandLineInterface.h>
#include <ccnx/forwarder/metis/config/metis_WebInterface.h>
#include <ccnx/forwarder/metis/processor/metis_MessageProcessor.h>

#include <ccnx/forwarder/metis/core/metis_Wldr.h>

#include <LongBow/runtime.h>

// the router's clock frequency (we now use the monotonic clock)
#define METISHZ 1000

// these will all be a little off because its all integer division
#define METIS_MSEC_PER_TICK (1000 / METISHZ)
#define METIS_USEC_PER_TICK (1000000 / METISHZ)
#define METIS_NSEC_PER_TICK ((1000000000ULL) / METISHZ)
#define MSEC_TO_TICKS(msec) ((msec < FC_MSEC_PER_TICK) ? 1 : msec / FC_MSEC_PER_TICK)
#define NSEC_TO_TICKS(nsec) ((nsec < METIS_NSEC_PER_TICK) ? 1 : nsec / METIS_NSEC_PER_TICK)


struct metis_forwarder {
    MetisDispatcher *dispatcher;

    uint16_t server_port;

    PARCEventSignal *signal_int;
    PARCEventSignal *signal_term;
    PARCEventSignal *signal_usr1;
    PARCEventTimer *keepalive_event;

    // This is added to metisForwarder_GetTime().  Some unit tests
    // will skew the virtual clock forward.  In normal operaiton, it is 0.
    MetisTicks clockOffset;

    unsigned nextConnectionid;
    MetisMessenger *messenger;
    MetisConnectionManager *connectionManager;
    MetisConnectionTable *connectionTable;
    MetisListenerSet *listenerSet;
    MetisConfiguration *config;

    // we'll eventually want to setup a threadpool of these
    MetisMessageProcessor *processor;

    MetisLogger *logger;

    PARCClock *clock;

    // used by seed48 and nrand48
    unsigned short seed[3];
};

// signal traps through the event scheduler
static void _signal_cb(int, PARCEventType, void *);

// A no-op keepalive to prevent Libevent from exiting the dispatch loop
static void _keepalive_cb(int, PARCEventType, void *);

/**
 * Reseed our pseudo-random number generator.
 */
static void
metisForwarder_Seed(MetisForwarder *metis)
{
    int fd;
    ssize_t res;

    res = -1;
    fd = open("/dev/urandom", O_RDONLY);
    if (fd != -1) {
        res = read(fd, metis->seed, sizeof(metis->seed));
        close(fd);
    }
    if (res != sizeof(metis->seed)) {
        metis->seed[1] = (unsigned short) getpid(); /* better than no entropy */
        metis->seed[2] = (unsigned short) time(NULL);
    }
    /*
     * The call to seed48 is needed by cygwin, and should be harmless
     * on other platforms.
     */
    seed48(metis->seed);
}

MetisLogger *
metisForwarder_GetLogger(const MetisForwarder *metis)
{
    return metis->logger;
}

// ============================================================================
// Setup and destroy section

MetisForwarder *
metisForwarder_Create(MetisLogger *logger)
{
    MetisForwarder *metis = parcMemory_AllocateAndClear(sizeof(MetisForwarder));
    assertNotNull(metis, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisForwarder));
    memset(metis, 0, sizeof(MetisForwarder));
    metisForwarder_Seed(metis);

    metis->clock = parcClock_Monotonic();
    metis->clockOffset = 0;

    if (logger) {
        metis->logger = metisLogger_Acquire(logger);
        metisLogger_SetClock(metis->logger, metis->clock);
    } else {
        PARCLogReporter *reporter = parcLogReporterTextStdout_Create();
        metis->logger = metisLogger_Create(reporter, metis->clock);
        parcLogReporter_Release(&reporter);
    }

    metis->nextConnectionid = 1;
    metis->dispatcher = metisDispatcher_Create(metis->logger);
    metis->messenger = metisMessenger_Create(metis->dispatcher);
    metis->connectionManager = metisConnectionManager_Create(metis);
    metis->connectionTable = metisConnectionTable_Create();
    metis->listenerSet = metisListenerSet_Create();
    metis->config = metisConfiguration_Create(metis);
    metis->processor = metisMessageProcessor_Create(metis);

    metis->signal_term = metisDispatcher_CreateSignalEvent(metis->dispatcher, _signal_cb, metis, SIGTERM);
    metisDispatcher_StartSignalEvent(metis->dispatcher, metis->signal_term);

    metis->signal_int = metisDispatcher_CreateSignalEvent(metis->dispatcher, _signal_cb, metis, SIGINT);
    metisDispatcher_StartSignalEvent(metis->dispatcher, metis->signal_int);

    metis->signal_usr1 = metisDispatcher_CreateSignalEvent(metis->dispatcher, _signal_cb, metis, SIGPIPE);
    metisDispatcher_StartSignalEvent(metis->dispatcher, metis->signal_usr1);

    /* ignore child */
    signal(SIGCHLD, SIG_IGN);

    /* ignore tty signals */
    signal(SIGTSTP, SIG_IGN);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);

    // We no longer use this for ticks, but we need to have at least one event schedule
    // to keep Libevent happy.

    struct timeval wtnow_timeout;
    timerclear(&wtnow_timeout);

    wtnow_timeout.tv_sec = 0;
    wtnow_timeout.tv_usec = 50000;  // 20 Hz keepalive

    PARCEventScheduler *base = metisDispatcher_GetEventScheduler(metis->dispatcher);
    metis->keepalive_event = parcEventTimer_Create(base, PARCEventType_Persist, _keepalive_cb, (void *) metis);
    parcEventTimer_Start(metis->keepalive_event, &wtnow_timeout);

    return metis;
}

void
metisForwarder_Destroy(MetisForwarder **metisPtr)
{
    assertNotNull(metisPtr, "Parameter must be non-null double pointer");
    assertNotNull(*metisPtr, "Parameter must dereference to non-null pointer");
    MetisForwarder *metis = *metisPtr;

    parcEventTimer_Destroy(&(metis->keepalive_event));

    metisListenerSet_Destroy(&(metis->listenerSet));
    metisConnectionManager_Destroy(&(metis->connectionManager));
    metisConnectionTable_Destroy(&(metis->connectionTable));
    metisMessageProcessor_Destroy(&(metis->processor));
    metisConfiguration_Destroy(&(metis->config));

    // the messenger is used by many of the other pieces, so destroy it last
    metisMessenger_Destroy(&(metis->messenger));

    metisDispatcher_DestroySignalEvent(metis->dispatcher, &(metis->signal_int));
    metisDispatcher_DestroySignalEvent(metis->dispatcher, &(metis->signal_term));
    metisDispatcher_DestroySignalEvent(metis->dispatcher, &(metis->signal_usr1));

    parcClock_Release(&metis->clock);
    metisLogger_Release(&metis->logger);

    // do the dispatcher last
    metisDispatcher_Destroy(&(metis->dispatcher));

    parcMemory_Deallocate((void **) &metis);
    *metisPtr = NULL;
}

void
metisForwarder_SetupAllListeners(MetisForwarder *metis, uint16_t port, const char *localPath)
{
    assertNotNull(metis, "Parameter must be non-null");

    metisConfigurationListeners_SetupAll(metis->config, port, localPath);
}

void
metisForwarder_SetupFromConfigFile(MetisForwarder *forwarder, const char *filename)
{
    MetisConfigurationFile *configFile = metisConfigurationFile_Create(forwarder, filename);
    if (configFile) {
        //metisConfigurationFile_ProcessForwardingStrategies(forwarder->config, configFile);
        metisConfigurationFile_Process(configFile);
        metisConfigurationFile_Release(&configFile);
    }
}

MetisConfiguration *
metisForwarder_GetConfiguration(MetisForwarder *metis)
{
    assertNotNull(metis, "Parameter must be non-null");
    return metis->config;
}

// ============================================================================

unsigned
metisForwarder_GetNextConnectionId(MetisForwarder *metis)
{
    assertNotNull(metis, "Parameter must be non-null");
    return metis->nextConnectionid++;
}

MetisMessenger *
metisForwarder_GetMessenger(MetisForwarder *metis)
{
    assertNotNull(metis, "Parameter must be non-null");
    return metis->messenger;
}

MetisDispatcher *
metisForwarder_GetDispatcher(MetisForwarder *metis)
{
    assertNotNull(metis, "Parameter must be non-null");
    return metis->dispatcher;
}

MetisConnectionTable *
metisForwarder_GetConnectionTable(MetisForwarder *metis)
{
    assertNotNull(metis, "Parameter must be non-null");
    return metis->connectionTable;
}

MetisListenerSet *
metisForwarder_GetListenerSet(MetisForwarder *metis)
{
    assertNotNull(metis, "Parameter must be non-null");
    return metis->listenerSet;
}

void
metisForwarder_SetChacheStoreFlag(MetisForwarder *metis, bool val)
{
    assertNotNull(metis, "Parameter must be non-null");
    metisMessageProcessor_SetCacheStoreFlag(metis->processor, val);
}

bool
metisForwarder_GetChacheStoreFlag(MetisForwarder *metis)
{
    assertNotNull(metis, "Parameter must be non-null");
    return metisMessageProcessor_GetCacheStoreFlag(metis->processor);
}

void
metisForwarder_SetChacheServeFlag(MetisForwarder *metis, bool val)
{
    assertNotNull(metis, "Parameter must be non-null");
    metisMessageProcessor_SetCacheServeFlag(metis->processor, val);
}

bool
metisForwarder_GetChacheServeFlag(MetisForwarder *metis)
{
    assertNotNull(metis, "Parameter must be non-null");
    return metisMessageProcessor_GetCacheServeFlag(metis->processor);
}

void
metisForwarder_Receive(MetisForwarder *metis, MetisMessage *message)
{
    assertNotNull(metis, "Parameter metis must be non-null");
    assertNotNull(message, "Parameter message must be non-null");

    // this takes ownership of the message, so we're done here
    if (metisMessage_GetType(message) == MetisMessagePacketType_Control) {
        metisConfiguration_Receive(metis->config, message);
    } else {
        const MetisConnection *conn = metisConnectionTable_FindById(metis->connectionTable, metisMessage_GetIngressConnectionId(message));
        if (metisConnection_HasWldr(conn)) {
            metisConnection_DetectLosses((MetisConnection *) conn, message);
        }
        if (metisMessage_HasWldr(message) && (metisMessage_GetWldrType(message) == WLDR_NOTIFICATION)) {
            //this is a wldr notification packet. We can discard it
            metisMessage_Release(&message);
            return;
        }
        metisMessageProcessor_Receive(metis->processor, message);
    }
}

MetisTicks
metisForwarder_GetTicks(const MetisForwarder *metis)
{
    assertNotNull(metis, "Parameter must be non-null");
    return parcClock_GetTime(metis->clock) + metis->clockOffset;
}

MetisTicks
metisForwarder_NanosToTicks(uint64_t nanos)
{
    return NSEC_TO_TICKS(nanos);
}

uint64_t
metisForwarder_TicksToNanos(MetisTicks ticks)
{
    return (1000000000ULL) * ticks / METISHZ;
}

bool
metisForwarder_AddOrUpdateRoute(MetisForwarder *metis, CPIRouteEntry *route)
{
    assertNotNull(metis, "Parameter metis must be non-null");
    assertNotNull(route, "Parameter route must be non-null");

    // we only have one message processor
    return metisMessageProcessor_AddOrUpdateRoute(metis->processor, route);
}

bool
metisForwarder_RemoveRoute(MetisForwarder *metis, CPIRouteEntry *route)
{
    assertNotNull(metis, "Parameter metis must be non-null");
    assertNotNull(route, "Parameter route must be non-null");

    // we only have one message processor
    return metisMessageProcessor_RemoveRoute(metis->processor, route);
}

void
metisForwarder_RemoveConnectionIdFromRoutes(MetisForwarder *metis, unsigned connectionId)
{
    assertNotNull(metis, "Parameter metis must be non-null");
    metisMessageProcessor_RemoveConnectionIdFromRoutes(metis->processor, connectionId);
}

void
metisForwarder_SetStrategy(MetisForwarder *metis, CCNxName *prefix, const char *strategy)
{
    assertNotNull(metis, "Parameter metis must be non-null");
    assertNotNull(prefix, "Parameter route must be non-null");

    if (strategy == NULL) {
        strategy = "random";
    }

    metisProcessor_SetStrategy(metis->processor, prefix, strategy);
}

void
metisForwarder_AddTap(MetisForwarder *metis, MetisTap *tap)
{
    metisMessageProcessor_AddTap(metis->processor, tap);
}

void
metisForwarder_RemoveTap(MetisForwarder *metis, MetisTap *tap)
{
    metisMessageProcessor_RemoveTap(metis->processor, tap);
}

MetisFibEntryList *
metisForwarder_GetFibEntries(MetisForwarder *metis)
{
    return metisMessageProcessor_GetFibEntries(metis->processor);
}

void
metisForwarder_SetContentObjectStoreSize(MetisForwarder *metis, size_t maximumContentStoreSize)
{
    metisMessageProcessor_SetContentObjectStoreSize(metis->processor, maximumContentStoreSize);
}

void
metisForwarder_ClearCache(MetisForwarder *metis)
{
    metisMessageProcessor_ClearCache(metis->processor);
}

PARCClock *
metisForwarder_GetClock(const MetisForwarder *metis)
{
    return metis->clock;
}

// =======================================================

static void
_signal_cb(int sig, PARCEventType events, void *user_data)
{
    MetisForwarder *metis = (MetisForwarder *) user_data;

    metisLogger_Log(metis->logger, MetisLoggerFacility_Core, PARCLogLevel_Warning, __func__,
                    "signal %d events %d", sig, events);

    switch ((int) sig) {
        case SIGTERM:
            metisLogger_Log(metis->logger, MetisLoggerFacility_Core, PARCLogLevel_Warning, __func__,
                            "Caught an terminate signal; exiting cleanly.");
            metisDispatcher_Stop(metis->dispatcher);
            break;

        case SIGINT:
            metisLogger_Log(metis->logger, MetisLoggerFacility_Core, PARCLogLevel_Warning, __func__,
                            "Caught an interrupt signal; exiting cleanly.");
            metisDispatcher_Stop(metis->dispatcher);
            break;

        case SIGUSR1:
            // dump stats
            break;

        default:
            break;
    }
}

static void
_keepalive_cb(int fd, PARCEventType what, void *user_data)
{
    assertTrue(what & PARCEventType_Timeout, "Got unexpected tick_cb: %d", what);
    // function is just a keepalive for Metis, does not do anything
}
