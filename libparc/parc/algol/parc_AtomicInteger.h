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
 * @file parc_AtomicInteger.h
 * @ingroup threading
 * @brief An integer value that may be updated automatically.
 *
 */
#ifndef libparc_parc_AtomicInteger_h
#define libparc_parc_AtomicInteger_h

#include <stdint.h>

/*
 * If we are compiling with GCC or Clang, then
 * we have some compiler extensions to make use of processer
 * compare-and-swap and other kinds of synchronisation primitives.
 */
#if defined(__GNUC__) || defined(__clang__)
# define USE_GCC_EXTENSIONS 1
#endif

// Turn this off until the problem on the SMP machines is worked out, case 787.
#undef USE_GCC_EXTENSIONS

#ifdef USE_GCC_EXTENSIONS
uint32_t parcAtomicInteger_Uint32IncrementGCC(uint32_t *value);

uint32_t parcAtomicInteger_Uint32DecrementGCC(uint32_t *value);

uint64_t parcAtomicInteger_Uint64IncrementGCC(uint64_t *value);

uint64_t parcAtomicInteger_Uint64DecrementGCC(uint64_t *value);

#define parcAtomicInteger_Uint32Increment parcAtomicInteger_Uint32IncrementGCC

#define parcAtomicInteger_Uint32Decrement parcAtomicInteger_Uint32DecrementGCC

#define parcAtomicInteger_Uint64Increment parcAtomicInteger_Uint64IncrementGCC

#define parcAtomicInteger_Uint64Decrement parcAtomicInteger_Uint64DecrementGCC

#else
uint32_t parcAtomicInteger_Uint32IncrementPthread(uint32_t *value);

uint32_t parcAtomicInteger_Uint32DecrementPthread(uint32_t *value);

uint64_t parcAtomicInteger_Uint64IncrementPthread(uint64_t *value);

uint64_t parcAtomicInteger_Uint64DecrementPthread(uint64_t *value);

#define parcAtomicInteger_Uint32Increment parcAtomicInteger_Uint32IncrementPthread

#define parcAtomicInteger_Uint32Decrement parcAtomicInteger_Uint32DecrementPthread

#define parcAtomicInteger_Uint64Increment parcAtomicInteger_Uint64IncrementPthread

#define parcAtomicInteger_Uint64Decrement parcAtomicInteger_Uint64DecrementPthread
#endif
#endif // libparc_parc_AtomicInteger_h
