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
#include <stdio.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>

#include <parc/concurrent/parc_Thread.h>

struct PARCThread {
    void *(*run)(PARCThread *, PARCObject *param);
    PARCObject *argument;
    bool isCancelled;
    bool isRunning;
    pthread_t thread;
};

static bool
_parcThread_Destructor(PARCThread **instancePtr)
{
    assertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCThread pointer.");
    PARCThread *thread = *instancePtr;

    if (thread->argument != NULL) {
        parcObject_Release(&thread->argument);
    }

    thread->isCancelled = true;
    parcThread_Join(thread);

    return true;
}

parcObject_ImplementAcquire(parcThread, PARCThread);

parcObject_ImplementRelease(parcThread, PARCThread);

parcObject_Override(PARCThread, PARCObject,
                    .isLockable = true,
                    .destructor = (PARCObjectDestructor *) _parcThread_Destructor,
                    .copy = (PARCObjectCopy *) parcThread_Copy,
                    .toString = (PARCObjectToString *) parcThread_ToString,
                    .equals = (PARCObjectEquals *) parcThread_Equals,
                    .compare = (PARCObjectCompare *) parcThread_Compare,
                    .hashCode = (PARCObjectHashCode *) parcThread_HashCode,
                    .display = (PARCObjectDisplay *) parcThread_Display
                    );

void
parcThread_AssertValid(const PARCThread *instance)
{
    assertTrue(parcThread_IsValid(instance),
               "PARCThread is not valid.");
}

PARCThread *
parcThread_Create(void *(*runFunction)(PARCThread *, PARCObject *), PARCObject *restrict parameter)
{
    assertNotNull(parameter, "Parameter cannot be NULL.");

    PARCThread *result = parcObject_CreateAndClearInstance(PARCThread);

    if (result) {
        result->run = runFunction;
        result->argument = parcObject_Acquire(parameter);
        result->isCancelled = false;
        result->isRunning = false;
    }

    return result;
}

int
parcThread_Compare(const PARCThread *instance, const PARCThread *other)
{
    int result = 0;
    return result;
}

PARCThread *
parcThread_Copy(const PARCThread *original)
{
    PARCThread *result = parcThread_Create(original->run, original->argument);
    result->isCancelled = original->isCancelled;
    result->isRunning = original->isRunning;

    return result;
}

void
parcThread_Display(const PARCThread *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCThread@%p {", instance);
    /* Call Display() functions for the fields here. */
    parcDisplayIndented_PrintLine(indentation, "}");
}

bool
parcThread_Equals(const PARCThread *x, const PARCThread *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        result = pthread_equal(x->thread, y->thread);
    }

    return result;
}

PARCHashCode
parcThread_HashCode(const PARCThread *instance)
{
    PARCHashCode result = 0;

    return result;
}

bool
parcThread_IsValid(const PARCThread *thread)
{
    bool result = false;

    if (thread != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
parcThread_ToJSON(const PARCThread *thread)
{
    PARCJSON *result = parcJSON_Create();

    return result;
}

char *
parcThread_ToString(const PARCThread *thread)
{
    char *result = parcMemory_Format("PARCThread@%p{.id=%p, .isCancelled=%s}", thread, thread->thread, thread->isCancelled ? "true" : "false");

    return result;
}

static void *
_parcThread_Run(PARCThread *thread)
{
    thread->isRunning = true;
    thread->run(thread, thread->argument);
    thread->isRunning = false;

    // The thread is done, release the reference to the argument acquired when this PARCThread was created.
    // This prevents the reference from lingering leading to memory leaks if the thread is not properly joined.
    if (thread->argument != NULL) {
        parcObject_Release(&thread->argument);
    }
    //  Release the thread reference that was acquired *before* this thread was started.
    parcThread_Release(&thread);

    return NULL;
}

void
parcThread_Start(PARCThread *thread)
{
    PARCThread *parameter = parcThread_Acquire(thread);
    pthread_create(&thread->thread, NULL, (void *(*)(void *))_parcThread_Run, parameter);
}

PARCObject *
parcThread_GetParameter(const PARCThread *thread)
{
    return thread->argument;
}

bool
parcThread_Cancel(PARCThread *thread)
{
    if (parcThread_Lock(thread)) {
        thread->isCancelled = true;
        parcThread_Notify(thread);
        parcThread_Unlock(thread);
    }
    return true;
}

int
parcThread_GetId(const PARCThread *thread)
{
    return (int) thread->thread;
}

bool
parcThread_IsRunning(const PARCThread *thread)
{
    return thread->isRunning;
}

bool
parcThread_IsCancelled(const PARCThread *thread)
{
    return thread->isCancelled;
}

void
parcThread_Join(PARCThread *thread)
{
    pthread_join(thread->thread, NULL);
}
