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
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <errno.h>

#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>

#define __STDC_FORMAT_MACROS
#include <inttypes.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>

#include "rta_Framework.h"
#include "rta_ConnectionTable.h"
#include "rta_Framework_Commands.h"

#ifndef DEBUG_OUTPUT
#define DEBUG_OUTPUT 0
#endif

// the thread function
static void *_rtaFramework_Run(void *ctx);

/**
 * Starts the worker thread.  Blocks until started
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void
rtaFramework_Start(RtaFramework *framework)
{
    pthread_attr_t attr;

    // ensure we're in the INIT state, then bump to STARTING
    // %%% LOCK
    rta_Framework_LockStatus(framework);
    if (framework->status == FRAMEWORK_INIT) {
        framework->status = FRAMEWORK_STARTING;
        rta_Framework_BroadcastStatus(framework);
        rta_Framework_UnlockStatus(framework);
        // %%% UNLOCK
    } else {
        RtaFrameworkStatus status = framework->status;
        rta_Framework_UnlockStatus(framework);
        // %%% UNLOCK
        assertTrue(0, "Invalid state, not FRAMEWORK_INIT, got %d", status);
        return;
    }


    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    if (pthread_create(&framework->thread, &attr, _rtaFramework_Run, framework) != 0) {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }

    if (DEBUG_OUTPUT) {
        printf("%s framework started %p\n", __func__, (void *) framework);
    }

    // wait for notificaiton from event thread
    rta_Framework_LockStatus(framework);
    while (framework->status == FRAMEWORK_INIT) {
        rta_Framework_WaitStatus(framework);
    }
    rta_Framework_UnlockStatus(framework);

    if (DEBUG_OUTPUT) {
        printf("%s framework running %p\n", __func__, (void *) framework);
    }
}

static void *
_rtaFramework_Run(void *ctx)
{
    RtaFramework *framework = (RtaFramework *) ctx;

    // %%% LOCK
    rta_Framework_LockStatus(framework);
    if (framework->status != FRAMEWORK_STARTING) {
        assertTrue(0, "Invalid state, expected before %d, got %d", FRAMEWORK_STARTING, framework->status);
        rta_Framework_UnlockStatus(framework);
        // %%% UNLOCK
        pthread_exit(NULL);
    }
    framework->status = FRAMEWORK_RUNNING;

    // Set our thread name, only used to diagnose a crash or in debugging
#if __APPLE__
    pthread_setname_np("RTA Framework");
#else
    pthread_setname_np(framework->thread, "RTA Framework");
#endif

    rta_Framework_BroadcastStatus(framework);
    rta_Framework_UnlockStatus(framework);
    // %%% UNLOCK

    if (DEBUG_OUTPUT) {
        const int bufferLength = 1024;
        char frameworkName[bufferLength];
        pthread_getname_np(framework->thread, frameworkName, bufferLength);
        printf("Framework thread running: '%s'\n", frameworkName);
    }

    // blocks
    parcEventScheduler_Start(framework->base, PARCEventSchedulerDispatchType_Blocking);

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s existed parcEventScheduler_Start\n", framework->clock_ticks, __func__);
    }

    // %%% LOCK
    rta_Framework_LockStatus(framework);
    framework->status = FRAMEWORK_SHUTDOWN;
    rta_Framework_BroadcastStatus(framework);
    rta_Framework_UnlockStatus(framework);
    // %%% UNLOCK

    pthread_exit(NULL);
}

/**
 * Stops the worker thread by sending a CommandShutdown.
 * Blocks until shutdown complete.
 *
 * CALLED FROM API's THREAD
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void
rtaFramework_Shutdown(RtaFramework *framework)
{
    RtaCommand *shutdown = rtaCommand_CreateShutdownFramework();
    rtaCommand_Write(shutdown, framework->commandRingBuffer);
    parcNotifier_Notify(framework->commandNotifier);
    rtaCommand_Release(&shutdown);

    // now block on reading status
    rtaFramework_WaitForStatus(framework, FRAMEWORK_SHUTDOWN);
}
