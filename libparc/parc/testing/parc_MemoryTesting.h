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
 * @file parc_TestingMemory.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */

#ifndef PARC_Library_parc_TestingMemory_h
#define PARC_Library_parc_TestingMemory_h

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>

/**
 * Determine if the current number of memory allocations is equal to the specified number.
 *
 * @param [in] expected The expected number of outstanding allocations.
 *
 * @return true The expected number of outstanding allocations is equal to the actual outstanding allocations.
 *
 * Example:
 * @code
 * {
 *     parcMemoryTesting_ExpectedOutstanding(0, "%s memory leak", __func__);
 * }
 * @endcode
 */
bool parcMemoryTesting_ExpectedOutstanding(const uint32_t expected, const char *format, ...);
#endif
