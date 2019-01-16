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

#include <parc/concurrent/parc_Synchronizer.h>

#ifdef PARCLibrary_DISABLE_ATOMICS
#  include <pthread.h>
#else
//#  include <pthread.h>
#endif

typedef enum {
    _PARCSynchronizer_Unlocked = 0,
    _PARCSynchronizer_Locked = 1
} _PARCSynchronizer;

struct PARCSynchronizer {
#ifdef PARCLibrary_DISABLE_ATOMICS
    pthread_mutex_t mutex;
#else
    int mutex;
#endif
};

static void
_parcSynchronizer_Finalize(PARCSynchronizer **instancePtr)
{
    parcAssertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCSynchronizer pointer.");

    parcSynchronizer_OptionalAssertValid((*instancePtr));
}

parcObject_ImplementAcquire(parcSynchronizer, PARCSynchronizer);

parcObject_ImplementRelease(parcSynchronizer, PARCSynchronizer);

parcObject_ExtendPARCObject(PARCSynchronizer, _parcSynchronizer_Finalize, NULL, NULL, NULL, NULL, NULL, NULL);

void
parcSynchronizer_AssertValid(const PARCSynchronizer *instance)
{
    parcAssertTrue(parcSynchronizer_IsValid(instance),
               "PARCSynchronizer is not valid.");
}

PARCSynchronizer *
parcSynchronizer_Create(void)
{
    PARCSynchronizer *result = parcObject_CreateInstance(PARCSynchronizer);

#ifdef PARCLibrary_DISABLE_ATOMICS
    pthread_mutex_init(&result->mutex, NULL);
#else
    result->mutex = _PARCSynchronizer_Unlocked;
#endif

    return result;
}

void
parcSynchronizer_Display(const PARCSynchronizer *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCSynchronizer@%p {", instance);
    /* Call Display() functions for the fields here. */
    parcDisplayIndented_PrintLine(indentation, "}");
}

bool
parcSynchronizer_IsValid(const PARCSynchronizer *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

bool
parcSynchronizer_TryLock(PARCSynchronizer *instance)
{
#ifdef PARCLibrary_DISABLE_ATOMICS
    bool result = pthread_mutex_trylock(&instance->mutex) == 0;
#else
    bool result = __sync_bool_compare_and_swap(&instance->mutex, _PARCSynchronizer_Unlocked, _PARCSynchronizer_Locked);
#endif
    return result;
}

void
parcSynchronizer_Lock(PARCSynchronizer *instance)
{
#ifdef PARCLibrary_DISABLE_ATOMICS
    pthread_mutex_lock(&instance->mutex);
#else
    while (!__sync_bool_compare_and_swap(&instance->mutex, _PARCSynchronizer_Unlocked, _PARCSynchronizer_Locked)) {
        ;
    }
#endif
}

void
parcSynchronizer_Unlock(PARCSynchronizer *instance)
{
#ifdef PARCLibrary_DISABLE_ATOMICS
    pthread_mutex_unlock(&instance->mutex);
#else
    while (!__sync_bool_compare_and_swap(&instance->mutex, _PARCSynchronizer_Locked, _PARCSynchronizer_Unlocked)) {
        ;
    }
#endif
}

bool
parcSynchronizer_IsLocked(const PARCSynchronizer *barrier)
{
#ifdef PARCLibrary_DISABLE_ATOMICS
    PARCSynchronizer *instance = (PARCSynchronizer *) barrier;

    bool result = pthread_mutex_trylock(&instance->mutex) != 0;
    pthread_mutex_unlock(&instance->mutex);
    return result;
#else
    return barrier->mutex == _PARCSynchronizer_Locked;
#endif
}
