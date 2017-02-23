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
 *  *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */

#include <config.h>
#include <stdio.h>
#include <unistd.h>

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

// This is implemented in rta_Framework_Commands
void
rtaFramework_DestroyProtocolHolder(RtaFramework *framework, FrameworkProtocolHolder *holder);

/**
 * If running in non-threaded mode (you don't call _Start), you must manually
 * turn the crank.  This turns it for a single cycle.
 * Return 0 on success, -1 on error (likely you're running in threaded mode)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int
rtaFramework_NonThreadedStep(RtaFramework *framework)
{
    if (framework->status == FRAMEWORK_INIT) {
        framework->status = FRAMEWORK_SETUP;
    }

    assertTrue(framework->status == FRAMEWORK_SETUP,
               "Framework invalid state for non-threaded, expected %d got %d",
               FRAMEWORK_SETUP,
               framework->status
               );

    if (framework->status != FRAMEWORK_SETUP) {
        return -1;
    }

    if (parcEventScheduler_Start(framework->base, PARCEventSchedulerDispatchType_LoopOnce) < 0) {
        return -1;
    }

    return 0;
}

/**
 * If running in non-threaded mode (you don't call _Start), you must manually
 * turn the crank.  This turns it for a number of cycles.
 * Return 0 on success, -1 on error (likely you're running in threaded mode)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int
rtaFramework_NonThreadedStepCount(RtaFramework *framework, unsigned count)
{
    if (framework->status == FRAMEWORK_INIT) {
        framework->status = FRAMEWORK_SETUP;
    }

    assertTrue(framework->status == FRAMEWORK_SETUP,
               "Framework invalid state for non-threaded, expected %d got %d",
               FRAMEWORK_SETUP,
               framework->status
               );

    if (framework->status != FRAMEWORK_SETUP) {
        return -1;
    }

    while (count-- > 0) {
        if (parcEventScheduler_Start(framework->base, PARCEventSchedulerDispatchType_LoopOnce) < 0) {
            return -1;
        }
    }
    return 0;
}

/**
 * If running in non-threaded mode (you don't call _Start), you must manually
 * turn the crank.  This turns it for a given amount of time.
 * Return 0 on success, -1 on error (likely you're running in threaded mode)
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int
rtaFramework_NonThreadedStepTimed(RtaFramework *framework, struct timeval *duration)
{
    if (framework->status == FRAMEWORK_INIT) {
        framework->status = FRAMEWORK_SETUP;
    }

    assertTrue(framework->status == FRAMEWORK_SETUP,
               "Framework invalid state for non-threaded, expected %d got %d",
               FRAMEWORK_SETUP,
               framework->status
               );

    if (framework->status != FRAMEWORK_SETUP) {
        return -1;
    }

    parcEventScheduler_Stop(framework->base, duration);

    if (parcEventScheduler_Start(framework->base, 0) < 0) {
        return -1;
    }
    return 0;
}


/**
 * After a protocol stack is created, you need to Teardown.  If you
 * are running in threaded mode (did a _Start), you should send an asynchronous
 * SHUTDOWN command instead.  This function only works if in the SETUP state
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
int
rtaFramework_Teardown(RtaFramework *framework)
{
    FrameworkProtocolHolder *holder;

    assertNotNull(framework, "called with null framework");

    if (DEBUG_OUTPUT) {
        printf("%9" PRIu64 " %s framework %p\n",
               rtaFramework_GetTicks(framework),
               __func__, (void *) framework);
    }

    // %%% LOCK
    rta_Framework_LockStatus(framework);
    if (framework->status != FRAMEWORK_SETUP) {
        RtaFrameworkStatus status = framework->status;
        rta_Framework_UnlockStatus(framework);
        // %%% UNLOCK
        assertTrue(0, "Invalid state, expected FRAMEWORK_SETUP, got %d", status);
        return -1;
    }

    holder = TAILQ_FIRST(&framework->protocols_head);
    while (holder != NULL) {
        FrameworkProtocolHolder *temp = TAILQ_NEXT(holder, list);
        rtaFramework_DestroyProtocolHolder(framework, holder);
        holder = temp;
    }

    framework->status = FRAMEWORK_TEARDOWN;
    rta_Framework_BroadcastStatus(framework);
    rta_Framework_UnlockStatus(framework);
    // %%% UNLOCK

    return 0;
}
