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
 */
#include <config.h>

#include <stdio.h>
#include <parc/testing/parc_MemoryTesting.h>
#include <parc/algol/parc_Memory.h>

bool
parcMemoryTesting_ExpectedOutstanding(const uint32_t expected, const char *format, ...)
{
    bool result = true;

    int allocationsLeaked = parcMemory_Outstanding() - expected;
    if (allocationsLeaked != 0) {
        va_list ap;
        va_start(ap, format);
        vprintf(format, ap);
        va_end(ap);
        if (allocationsLeaked < 0) {
            printf(" (%d more allocations deallocated than allocated)\n", -allocationsLeaked);
        } else {
            printf(" (%d allocations not deallocated)\n", allocationsLeaked);
        }
        result = false;
    }

    return result;
}
