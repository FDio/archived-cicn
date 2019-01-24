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

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/concurrent/parc_AtomicUint64.h>

struct PARCAtomicUint64 {
    uint64_t value;
#ifdef PARCLibrary_DISABLE_ATOMICS
    pthread_mutex_t mutex;
#endif
};

static void
_parcAtomicUint64_Finalize(PARCAtomicUint64 **instancePtr)
{
    parcAssertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCAtomicUint64 pointer.");

    parcAtomicUint64_OptionalAssertValid((*instancePtr));

    /* cleanup the instance fields here */
}

parcObject_ImplementAcquire(parcAtomicUint64, PARCAtomicUint64);

parcObject_ImplementRelease(parcAtomicUint64, PARCAtomicUint64);

parcObject_ExtendPARCObject(PARCAtomicUint64, _parcAtomicUint64_Finalize, parcAtomicUint64_Copy, NULL, parcAtomicUint64_Equals, parcAtomicUint64_Compare, parcAtomicUint64_HashCode, NULL);


void
parcAtomicUint64_AssertValid(const PARCAtomicUint64 *instance)
{
    parcAssertTrue(parcAtomicUint64_IsValid(instance),
               "PARCAtomicUint64 is not valid.");
}

PARCAtomicUint64 *
parcAtomicUint64_Create(uint64_t value)
{
    PARCAtomicUint64 *result = parcObject_CreateAndClearInstance(PARCAtomicUint64);

#ifdef PARCLibrary_DISABLE_ATOMICS
    pthread_mutex_init(&result->mutex, NULL);
    result->value = value;
#else
    *result = value;
#endif

    return result;
}

int
parcAtomicUint64_Compare(const PARCAtomicUint64 *instance, const PARCAtomicUint64 *other)
{
    int64_t comparison = parcAtomicUint64_GetValue(instance) - parcAtomicUint64_GetValue(other);

    int result = 0;
    if (comparison < 0) {
        result = -1;
    } else if (comparison > 0) {
        result = 1;
    }

    return result;
}

PARCAtomicUint64 *
parcAtomicUint64_Copy(const PARCAtomicUint64 *original)
{
    PARCAtomicUint64 *result = parcAtomicUint64_Create(parcAtomicUint64_GetValue(original));

    return result;
}

bool
parcAtomicUint64_Equals(const PARCAtomicUint64 *x, const PARCAtomicUint64 *y)
{
    bool result = false;

    result = parcAtomicUint64_GetValue(x) == parcAtomicUint64_GetValue(y);

    return result;
}

PARCHashCode
parcAtomicUint64_HashCode(const PARCAtomicUint64 *instance)
{
    PARCHashCode result = (PARCHashCode) parcAtomicUint64_GetValue(instance);

    return result;
}

bool
parcAtomicUint64_IsValid(const PARCAtomicUint64 *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

uint64_t
parcAtomicUint64_GetValue(const PARCAtomicUint64 *instance)
{
#ifdef PARCLibrary_DISABLE_ATOMICS
    return instance->value;
#else
    return *instance;
#endif
}

uint64_t
parcAtomicUint64_AddImpl(PARCAtomicUint64 *value, uint64_t addend)
{
#ifdef PARCLibrary_DISABLE_ATOMICS
    pthread_mutex_lock(&value->mutex);
    value->value += addend;
    uint64_t result = value->value;
    pthread_mutex_unlock(&value->mutex);
    return result;
#else
    return __sync_add_and_fetch(value, addend);
#endif
}

uint64_t
parcAtomicUint64_SubtractImpl(PARCAtomicUint64 *value, uint64_t subtrahend)
{
#ifdef PARCLibrary_DISABLE_ATOMICS
    pthread_mutex_lock(&value->mutex);
    value->value -= subtrahend;
    uint64_t result = value->value;
    pthread_mutex_unlock(&value->mutex);
    return result;
#else
    return __sync_sub_and_fetch(value, subtrahend);
#endif
}

bool
parcAtomicUint64_CompareAndSwapImpl(PARCAtomicUint64 *value, uint64_t predicate, uint64_t newValue)
{
    bool result = false;
#ifdef PARCLibrary_DISABLE_ATOMICS
    pthread_mutex_lock(&value->mutex);
    if (value->value == predicate) {
        value->value = newValue;
        result = true;
    }
    pthread_mutex_unlock(&value->mutex);
    return result;
#else
    result = __sync_bool_compare_and_swap(value, predicate, newValue);
#endif
    return result;
}
