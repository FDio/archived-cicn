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
#include <pthread.h>

#include <parc/algol/parc_AtomicInteger.h>

#ifdef USE_GCC_EXTENSIONS
uint32_t
parcAtomicInteger_Uint32IncrementGCC(uint32_t *value)
{
    return __sync_add_and_fetch(value, 1);
}

uint32_t
parcAtomicInteger_Uint32DecrementGCC(uint32_t *value)
{
    return __sync_sub_and_fetch(value, 1);
}

uint64_t
parcAtomicInteger_Uint64IncrementGCC(uint64_t *value)
{
    return __sync_add_and_fetch(value, 1);
}

uint64_t
parcAtomicInteger_Uint64DecrementGCC(uint64_t *value)
{
    return __sync_sub_and_fetch(value, 1);
}
#endif


// Since there is no per integer mutex, we must use something to protect an integer from multiple,
// simultaneous competitors.
// Unfortunately, this is an indiscrimiate mutex causing all atomic integer updates to be
// serialized rather than each individually protected.
static pthread_mutex_t _parcAtomicInteger_PthreadMutex = PTHREAD_MUTEX_INITIALIZER;

uint32_t
parcAtomicInteger_Uint32IncrementPthread(uint32_t *value)
{
    pthread_mutex_lock(&_parcAtomicInteger_PthreadMutex);
    *value = *value + 1;
    uint32_t result = *value;
    pthread_mutex_unlock(&_parcAtomicInteger_PthreadMutex);
    return result;
}

uint32_t
parcAtomicInteger_Uint32DecrementPthread(uint32_t *value)
{
    pthread_mutex_lock(&_parcAtomicInteger_PthreadMutex);
    *value = *value - 1;
    uint32_t result = *value;
    pthread_mutex_unlock(&_parcAtomicInteger_PthreadMutex);
    return result;
}

uint64_t
parcAtomicInteger_Uint64IncrementPthread(uint64_t *value)
{
    pthread_mutex_lock(&_parcAtomicInteger_PthreadMutex);
    *value = *value + 1;
    uint64_t result = *value;
    pthread_mutex_unlock(&_parcAtomicInteger_PthreadMutex);
    return result;
}

uint64_t
parcAtomicInteger_Uint64DecrementPthread(uint64_t *value)
{
    pthread_mutex_lock(&_parcAtomicInteger_PthreadMutex);
    *value = *value - 1;
    uint64_t result = *value;
    pthread_mutex_unlock(&_parcAtomicInteger_PthreadMutex);
    return result;
}
