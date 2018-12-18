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
 * @file tests.h
 * @brief LongBow Utility Tests
 *
 *
 */
#ifndef LongBow_tests_h
#define LongBow_tests_h

#include <stdbool.h>
#include <stdlib.h>

/**
 * Test if an address is aligned.
 *
 * Return true of the given address is aligned according to alignment.
 * The value for alignment must be a power of 2.
 *
 * @param [in] address The adddress to test.
 * @param [in] alignment A power of 2 greater than or equal to sizeof(void *).
 *
 * @return true if the address is aligned.
 * @return false if the address is not aligned.
 */
bool longBowRuntime_TestAddressIsAligned(const void *address, size_t alignment);

/**
 * Induce a core-dump
 *
 */
void longBowRuntime_CoreDump(void);
#endif // LongBow_tests_h
