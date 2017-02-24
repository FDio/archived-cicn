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
 * @file rta_Framework.h
 * @brief <#Brief Description#>
 *
 * rtaFramework executes inside the worker thread in callback from the event scheduler.
 *
 * It provides service functions to components and connectors so they do not need
 * to be event aware.
 *
 * It also manages the command channel to communicate with rtaTransport in the API's thread.
 *
 * _Create(), _Start(), and _Destroy() are called from the API's thread.  You should not
 * call _Destroy until rtaFramework_GetStatus() is FRAMEWORK_SHUTDOWN.
 *
 * The framework can run in threaded mode or non-threaded mode.  Including this one
 * header gives you both sets of operations, but they are not compatible.
 *
 * THREADED MODE:
 *      call _Create
 *      call _Start
 *      ... do work ...
 *      call _Shutdown
 *      call _Destroy
 *
 * NON-THREADED MODE
 *      call _Create
 *      ... do work ...
 *      call _Step or _StepCount or _StepTimed
 *      ... do work ...
 *      call _Step or _StepCount or _StepTimed
 *      ... do work ...
 *      call _Teardown
 *      call _Destroy
 *
 */
#ifndef Libccnx_rta_Framework_h
#define Libccnx_rta_Framework_h

#include <parc/concurrent/parc_RingBuffer_1x1.h>
#include <parc/concurrent/parc_Notifier.h>
#include <ccnx/transport/transport_rta/core/rta_Logger.h>

// ===================================
// External API, used by rtaTransport

struct rta_framework;
typedef struct rta_framework RtaFramework;

#define RTA_MAX_PRIORITY    0
#define RTA_NORMAL_PRIORITY 1
#define RTA_MIN_PRIORITY    2

/**
 * Transient states: STARTING, STOPPING.  You don't want to block waiting for those
 * as you could easily miss them
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
typedef enum {
    FRAMEWORK_INIT = 0,       /** Initial status after Create() *
                               * Example:
                               * @code
                               * <#example#>
                               * @endcode
                               */
    FRAMEWORK_SETUP = 1,      /** Configured in non-threaded mode *
                               * Example:
                               * @code
                               * <#example#>
                               * @endcode
                               */

    FRAMEWORK_STARTING = 2,   /** Between calling _Start() and the thread running *
                               * Example:
                               * @code
                               * <#example#>
                               * @endcode
                               */
    FRAMEWORK_RUNNING = 3,    /** After event scheduler thread starts *
                               * Example:
                               * @code
                               * <#example#>
                               * @endcode
                               */
    FRAMEWORK_STOPPING = 4,   /** When shutdown is finished, but before event scheduler exists *
                               * Example:
                               * @code
                               * <#example#>
                               * @endcode
                               */

    FRAMEWORK_TEARDOWN = 5,   /** After cleanup from SETUP *
                               * Example:
                               * @code
                               * <#example#>
                               * @endcode
                               */
    FRAMEWORK_SHUTDOWN = 6,   /** After event scheduler exits *
                               * Example:
                               * @code
                               * <#example#>
                               * @endcode
                               */
} RtaFrameworkStatus;

/**
 * Creates the framework context, but does not start the worker thread.
 * <code>command_fd</code> is the socketpair or pipe (one-way is ok) over which
 * RTATransport will send commands.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaFramework *rtaFramework_Create(PARCRingBuffer1x1 *commandRingBuffer, PARCNotifier *commandNotifier);


void rtaFramework_Destroy(RtaFramework **frameworkPtr);

/**
 * Returns the Logging system used by the framework
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] framework An allocated RtaFramework
 *
 * @retval non-null The Logging system
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaLogger *rtaFramework_GetLogger(RtaFramework *framework);

/**
 * May block briefly, returns the current status of the framework.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaFrameworkStatus rtaFramework_GetStatus(RtaFramework *framework);

/**
 * Blocks until the framework status equals or exeeds the desired status
 * Transient states: STARTING, STOPPING.  You don't want to block waiting for those
 * as you could easily miss them
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
RtaFrameworkStatus rtaFramework_WaitForStatus(RtaFramework *framework,
                                              RtaFrameworkStatus status);


#include "rta_Framework_Threaded.h"
#include "rta_Framework_NonThreaded.h"
#endif
