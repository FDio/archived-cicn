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
 * @file rta_Framework_Threaded.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef Libccnx_rta_Framework_Threaded_h
#define Libccnx_rta_Framework_Threaded_h

// =============================
// THREADED

/**
 * Starts the worker thread.  Blocks until started.
 *
 * CALLED FROM API's THREAD
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void rtaFramework_Start(RtaFramework *framework);

/**
 * Stops the worker thread by sending a CommandShutdown.
 * Blocks until shutdown complete.
 *
 * The caller must provider their side of the command channel
 *
 * CALLED FROM API's THREAD
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void rtaFramework_Shutdown(RtaFramework *framework);

#endif
