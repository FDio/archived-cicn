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
#include <errno.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/concurrent/parc_Lock.h>

struct PARCLock {
    pthread_mutex_t lock;
    pthread_mutexattr_t lockAttributes;
    pthread_cond_t notification;
    bool locked;

    bool notified;
};

static void
_parcLock_Finalize(PARCLock **instancePtr)
{
    parcAssertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCLock pointer.");

    parcLock_OptionalAssertValid(*instancePtr);

    /* cleanup the instance fields here */
}

parcObject_ImplementAcquire(parcLock, PARCLock);

parcObject_ImplementRelease(parcLock, PARCLock);

parcObject_ExtendPARCObject(PARCLock, _parcLock_Finalize, NULL, parcLock_ToString, NULL, NULL, NULL, NULL);

void
parcLock_AssertValid(const PARCLock *instance)
{
    parcAssertTrue(parcLock_IsValid(instance),
               "PARCLock is not valid.");
}

PARCLock *
parcLock_Create(void)
{
    PARCLock *result = parcObject_CreateInstance(PARCLock);

    pthread_mutexattr_init(&result->lockAttributes);
    pthread_mutexattr_settype(&result->lockAttributes, PTHREAD_MUTEX_ERRORCHECK);

    pthread_mutex_init(&result->lock, &result->lockAttributes);

    result->locked = false;
    pthread_cond_init(&result->notification, NULL);
    result->notified = false;
    return result;
}

void
parcLock_Display(const PARCLock *lock, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCLock@%p {", lock);
    parcDisplayIndented_PrintLine(indentation + 1, ".locked=%s", lock->locked ? "true" : "false");

    parcDisplayIndented_PrintLine(indentation, "}");
}

bool
parcLock_IsValid(const PARCLock *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

PARCBufferComposer *
parcLock_BuildString(const PARCLock *lock, PARCBufferComposer *composer)
{
    parcBufferComposer_Format(composer, "lock{.state=%s }", lock->locked ? "true" : "false");

    return composer;
}

char *
parcLock_ToString(const PARCLock *lock)
{
    char *result = NULL;

    PARCBufferComposer *composer = parcBufferComposer_Create();
    if (composer != NULL) {
        parcLock_BuildString(lock, composer);
        result = parcBufferComposer_ToString(composer);
        parcBufferComposer_Release(&composer);
    }

    return result;
}

bool
parcLock_Unlock(PARCLock *lock)
{
    bool result = false;

    parcLock_OptionalAssertValid(lock);

    if (lock->locked) {
        result = (pthread_mutex_unlock(&lock->lock) == 0);
    }

    return result;
}

bool
parcLock_Lock(PARCLock *lock)
{
    bool result = false;

    parcLock_OptionalAssertValid(lock);

    int error = pthread_mutex_lock(&lock->lock);

    if (error == 0) {
        lock->locked = true;
        result = true;
        errno = 0;
    } else {
        errno = error;
    }

    return result;
}

bool
parcLock_TryLock(PARCLock *lock)
{
    bool result = false;

    parcLock_OptionalAssertValid(lock);

    result = (pthread_mutex_trylock(&lock->lock) == 0);
    if (result) {
        lock->locked = true;
    }

    return result;
}

bool
parcLock_IsLocked(const PARCLock *lock)
{
    parcLock_OptionalAssertValid(lock);

    return lock->locked;
}

void
parcLock_Wait(PARCLock *lock)
{
    parcLock_OptionalAssertValid(lock);

    parcTrapUnexpectedStateIf(lock->locked == false,
                          "You must Lock the object before calling parcLock_Wait");

    lock->notified = false;
    while (lock->notified == false) {
        pthread_cond_wait(&lock->notification, &lock->lock);
    }
}

void
parcLock_Notify(PARCLock *lock)
{
    parcLock_OptionalAssertValid(lock);

    parcTrapUnexpectedStateIf(lock->locked == false,
                          "You must Lock the object before calling parcLock_Notify");

    lock->notified = true;
    pthread_cond_signal(&lock->notification);
}

