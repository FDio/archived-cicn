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
 * @file rta_Framework_NonThreaded.h
 * @brief Implementation of the non-threaded api.
 *
 * Unless you call one of the _Step methods frequently, the tick clock will be off.
 *
 */
#ifndef Libccnx_rta_Framework_NonThreaded_h
#define Libccnx_rta_Framework_NonThreaded_h

#include <sys/time.h>

// ==============================
// NON-THREADED API

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
int rtaFramework_NonThreadedStep(RtaFramework *framework);

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
int rtaFramework_NonThreadedStepCount(RtaFramework *framework, unsigned count);

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
int rtaFramework_NonThreadedStepTimed(RtaFramework *framework, struct timeval *duration);


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
int rtaFramework_Teardown(RtaFramework *framework);
#endif
