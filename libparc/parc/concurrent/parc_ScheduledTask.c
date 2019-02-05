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

#ifndef _WIN32
#include <sys/time.h>
#endif

#include <config.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Time.h>
#include <parc/concurrent/parc_ScheduledTask.h>
#include <parc/concurrent/parc_FutureTask.h>

struct PARCScheduledTask {
    PARCFutureTask *task;
    uint64_t executionTime;
};

static bool
_parcScheduledTask_Destructor(PARCScheduledTask **instancePtr)
{
    parcAssertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCScheduledTask pointer.");
    PARCScheduledTask *task = *instancePtr;

    parcFutureTask_Release(&task->task);
    return true;
}

parcObject_ImplementAcquire(parcScheduledTask, PARCScheduledTask);

parcObject_ImplementRelease(parcScheduledTask, PARCScheduledTask);

parcObject_Override(PARCScheduledTask, PARCObject,
                    .isLockable = true,
                    .destructor = (PARCObjectDestructor *) _parcScheduledTask_Destructor,
                    .copy = (PARCObjectCopy *) parcScheduledTask_Copy,
                    .toString = (PARCObjectToString *) parcScheduledTask_ToString,
                    .equals = (PARCObjectEquals *) parcScheduledTask_Equals,
                    .compare = (PARCObjectCompare *) parcScheduledTask_Compare,
                    .hashCode = (PARCObjectHashCode *) parcScheduledTask_HashCode,
                    .display = (PARCObjectDisplay *) parcScheduledTask_Display);

void
parcScheduledTask_AssertValid(const PARCScheduledTask *instance)
{
    parcAssertTrue(parcScheduledTask_IsValid(instance),
               "PARCScheduledTask is not valid.");
}


PARCScheduledTask *
parcScheduledTask_Create(PARCFutureTask *task, uint64_t executionTime)
{
    PARCScheduledTask *result = parcObject_CreateInstance(PARCScheduledTask);

    if (result != NULL) {
        result->task = parcFutureTask_Acquire(task);
        result->executionTime = executionTime;
    }

    return result;
}

int
parcScheduledTask_Compare(const PARCScheduledTask *instance, const PARCScheduledTask *other)
{
    int result = 0;

    return result;
}

PARCScheduledTask *
parcScheduledTask_Copy(const PARCScheduledTask *original)
{
    PARCScheduledTask *result = parcScheduledTask_Create(original->task, original->executionTime);

    return result;
}

void
parcScheduledTask_Display(const PARCScheduledTask *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCScheduledTask@%p {", instance);
    /* Call Display() functions for the fields here. */
    parcDisplayIndented_PrintLine(indentation, "}");
}

bool
parcScheduledTask_Equals(const PARCScheduledTask *x, const PARCScheduledTask *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        if (parcFutureTask_Equals(x->task, y->task)) {
            if (x->executionTime == y->executionTime) {
                result = true;
            }
        }
    }

    return result;
}

PARCHashCode
parcScheduledTask_HashCode(const PARCScheduledTask *instance)
{
    PARCHashCode result = 0;

    return result;
}

bool
parcScheduledTask_IsValid(const PARCScheduledTask *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
parcScheduledTask_ToJSON(const PARCScheduledTask *instance)
{
    PARCJSON *result = parcJSON_Create();

    if (result != NULL) {
    }

    return result;
}

char *
parcScheduledTask_ToString(const PARCScheduledTask *instance)
{
    char *result = parcMemory_Format("PARCScheduledTask@%p\n", instance);

    return result;
}

uint64_t
parcScheduledTask_GetExecutionTime(const PARCScheduledTask *task)
{
    return task->executionTime;
}

bool
parcScheduledTask_Cancel(PARCScheduledTask *task, bool mayInterruptIfRunning)
{
    return parcFutureTask_Cancel(task->task, mayInterruptIfRunning);
}

PARCFutureTaskResult
parcScheduledTask_Get(const PARCScheduledTask *task, const PARCTimeout *timeout)
{
    return parcFutureTask_Get(task->task, timeout);
}

PARCFutureTask *
parcScheduledTask_GetTask(const PARCScheduledTask *task)
{
    return task->task;
}

void *
parcScheduledTask_Run(const PARCScheduledTask *task)
{
    return parcFutureTask_Run(task->task);
}

bool
parcScheduledTask_IsCancelled(const PARCScheduledTask *task)
{
    return parcFutureTask_IsCancelled(task->task);
}

bool
parcScheduledTask_IsDone(const PARCScheduledTask *task)
{
    return parcFutureTask_IsDone(task->task);
}
