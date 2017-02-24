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
 * @header Metis Threaded Forwarder
 * @abstract A thread wrapper around metis_Forwarder.
 * @discussion
 *     Cannot restart a thread after its stopped.  I think this should be ok, but
 *     have not had time to test it yet, so dont support it.
 *
 *     This wrapper does not expose any of the metis_Forwarder calls, as those
 *     are all non-threaded calls.  You can only create, start, stop, and destroy
 *     the forwarder.  All configuration needs to be via the CLI or via CPI control messages.
 *
 *     You may run multiple Metis forwarders as long as they are on different ports.
 *
 */

#include <config.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <parc/algol/parc_Memory.h>
#include <LongBow/runtime.h>

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/core/metis_ThreadedForwarder.h>
#include <ccnx/forwarder/metis/core/metis_Dispatcher.h>
#include <ccnx/forwarder/metis/config/metis_Configuration.h>
#include <ccnx/forwarder/metis/config/metis_CommandLineInterface.h>

struct metis_threaded_forwarder {
    pthread_t thread;
    pthread_mutex_t state_mutex;
    pthread_cond_t state_cond;

    // indicates that the Start function was called
    bool started;

    // indicates that the thread has entered the Run function and is running
    bool running;

    MetisForwarder *forwarder;
    MetisLogger *logger;
    MetisCommandLineInterface *cli;
};

static void
metisThreadedForwarder_LockState(MetisThreadedForwarder *threadedMetis)
{
    int res = pthread_mutex_lock(&threadedMetis->state_mutex);
    assertTrue(res == 0, "error from pthread_mutex_lock: %d", res);
}

static void
metisThreadedForwarder_UnlockState(MetisThreadedForwarder *threadedMetis)
{
    int res = pthread_mutex_unlock(&threadedMetis->state_mutex);
    assertTrue(res == 0, "error from pthread_mutex_unlock: %d", res);
}

static void
metisThreadedForwarder_WaitStatus(MetisThreadedForwarder *threadedMetis)
{
    int res = pthread_cond_wait(&threadedMetis->state_cond, &threadedMetis->state_mutex);
    assertTrue(res == 0, "error from pthread_mutex_unlock: %d", res);
}

static void
metisThreadedForwarder_BroadcastStatus(MetisThreadedForwarder *threadedMetis)
{
    int res = pthread_cond_broadcast(&threadedMetis->state_cond);
    assertTrue(res == 0, "error from pthread_mutex_unlock: %d", res);
}

static void *
metisThreadedForwarder_Run(void *arg)
{
    MetisThreadedForwarder *threadedMetis = (MetisThreadedForwarder *) arg;

    metisThreadedForwarder_LockState(threadedMetis);
    assertFalse(threadedMetis->running, "Invalid State: forwarder already in running state");
    threadedMetis->running = true;
    metisThreadedForwarder_BroadcastStatus(threadedMetis);
    metisThreadedForwarder_UnlockState(threadedMetis);

    // --------
    // Block in the dispatch loop
    MetisDispatcher *dispatcher = metisForwarder_GetDispatcher(threadedMetis->forwarder);
    metisDispatcher_Run(dispatcher);
    // --------

    metisThreadedForwarder_LockState(threadedMetis);
    assertTrue(threadedMetis->running, "Invalid State: forwarder indicates its not running!");
    threadedMetis->running = false;
    metisThreadedForwarder_BroadcastStatus(threadedMetis);
    metisThreadedForwarder_UnlockState(threadedMetis);

    pthread_exit(NULL);
}

// ===========================

MetisThreadedForwarder *
metisThreadedForwarder_Create(MetisLogger *logger)
{
    struct sigaction ignore_action;
    ignore_action.sa_handler = SIG_IGN;
    sigemptyset(&ignore_action.sa_mask);
    ignore_action.sa_flags = 0;
    //    sigaction(SIGPIPE, NULL, &save_sigpipe);
    sigaction(SIGPIPE, &ignore_action, NULL);


    MetisThreadedForwarder *threadedMetis = parcMemory_AllocateAndClear(sizeof(MetisThreadedForwarder));
    assertNotNull(threadedMetis, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisThreadedForwarder));
    threadedMetis->logger = metisLogger_Acquire(logger);
    threadedMetis->forwarder = metisForwarder_Create(logger);

    pthread_mutex_init(&threadedMetis->state_mutex, NULL);
    pthread_cond_init(&threadedMetis->state_cond, NULL);

    threadedMetis->thread = (pthread_t) { 0 };
    threadedMetis->cli = NULL;
    threadedMetis->running = false;
    return threadedMetis;
}

void
metisThreadedForwarder_AddCLI(MetisThreadedForwarder *threadedMetis, uint16_t port)
{
    assertNotNull(threadedMetis, "Parameter must be non-null");
    assertFalse(threadedMetis->started, "Must be done prior to starting!");
    assertNull(threadedMetis->cli, "Can only define one CLI");

    threadedMetis->cli = metisCommandLineInterface_Create(threadedMetis->forwarder, port);

    // this sets up all the network events in the dispatcher so when the thread is
    // started, the CLI will be ready to go.
    metisCommandLineInterface_Start(threadedMetis->cli);
}

void
metisThreadedForwarder_SetupAllListeners(MetisThreadedForwarder *threadedMetis, uint16_t port, const char *localPath)
{
    assertNotNull(threadedMetis, "Parameter must be non-null");
    assertFalse(threadedMetis->started, "Must be done prior to starting!");

    metisForwarder_SetupAllListeners(threadedMetis->forwarder, port, localPath);
}

void
metisThreadedForwarder_Start(MetisThreadedForwarder *threadedMetis)
{
    assertNotNull(threadedMetis, "Parameter must be non-null");
    assertFalse(threadedMetis->started, "Must be done prior to starting!");

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    int failure = pthread_create(&threadedMetis->thread, &attr, metisThreadedForwarder_Run, threadedMetis);
    assertFalse(failure, "Eror creating thread: %d", failure);

    // block until running
    metisThreadedForwarder_LockState(threadedMetis);
    while (!threadedMetis->running) {
        metisThreadedForwarder_WaitStatus(threadedMetis);
    }
    metisThreadedForwarder_UnlockState(threadedMetis);
}

/**
 * @function metisThreadedForwarder_Stop
 * @abstract Blocks until stopped
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
void
metisThreadedForwarder_Stop(MetisThreadedForwarder *threadedMetis)
{
    assertNotNull(threadedMetis, "Parameter must be non-null");

    // These are explicitly thread-safe operations inside Metis
    MetisDispatcher *dispatcher = metisForwarder_GetDispatcher(threadedMetis->forwarder);
    metisDispatcher_Stop(dispatcher);

    // Equivalently, we could block until joined

    // block until stopped
    metisThreadedForwarder_LockState(threadedMetis);
    while (threadedMetis->running) {
        metisThreadedForwarder_WaitStatus(threadedMetis);
    }
    metisThreadedForwarder_UnlockState(threadedMetis);
}

/**
 * @function metisThreadedForwarder_Destroy
 * @abstract Blocks until stopped and destoryed
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
void
metisThreadedForwarder_Destroy(MetisThreadedForwarder **threadedMetisPtr)
{
    assertNotNull(threadedMetisPtr, "Parameter must be non-null double pointer");
    assertNotNull(*threadedMetisPtr, "Parameter must dereference to non-null pointer");

    MetisThreadedForwarder *threadedMetis = *threadedMetisPtr;
    metisThreadedForwarder_Stop(threadedMetis);

    pthread_mutex_destroy(&threadedMetis->state_mutex);
    pthread_cond_destroy(&threadedMetis->state_cond);

    metisLogger_Release(&threadedMetis->logger);
    metisForwarder_Destroy(&threadedMetis->forwarder);
    parcMemory_Deallocate((void **) &threadedMetis);
    *threadedMetisPtr = NULL;
}
