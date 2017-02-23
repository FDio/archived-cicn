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

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Execution.h>

#include <parc/concurrent/parc_FutureTask.h>

struct PARCFutureTask {
    void *(*function)(PARCFutureTask *task, void *parameter);
    void *parameter;
    void *result;
    bool isRunning;
    bool isDone;
    bool isCancelled;
};

static void
_parcFutureTask_Initialise(PARCFutureTask *futureTask)
{
    futureTask->result = NULL;
    futureTask->isDone = false;
    futureTask->isCancelled = false;
    futureTask->isRunning = false;
}

static bool
_parcFutureTask_Destructor(PARCFutureTask **instancePtr)
{
    assertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCFutureTask pointer.");
    PARCFutureTask *task = *instancePtr;

    if (parcObject_IsInstanceOf(task->parameter, &PARCObject_Descriptor)) {
        parcObject_Release(&task->parameter);
    }

    return true;
}

parcObject_ImplementAcquire(parcFutureTask, PARCFutureTask);

parcObject_ImplementRelease(parcFutureTask, PARCFutureTask);

parcObject_Override(PARCFutureTask, PARCObject,
                    .isLockable = true,
                    .destructor = (PARCObjectDestructor *) _parcFutureTask_Destructor,
                    .copy = (PARCObjectCopy *) parcFutureTask_Copy,
                    .toString = (PARCObjectToString *) parcFutureTask_ToString,
                    .equals = (PARCObjectEquals *) parcFutureTask_Equals,
                    .compare = (PARCObjectCompare *) parcFutureTask_Compare,
                    .hashCode = (PARCObjectHashCode *) parcFutureTask_HashCode,
                    .display = (PARCObjectDisplay *) parcFutureTask_Display);

void
parcFutureTask_AssertValid(const PARCFutureTask *task)
{
    assertTrue(parcFutureTask_IsValid(task),
               "PARCFutureTask is not valid.");
}

PARCFutureTask *
parcFutureTask_Create(void *(*function)(PARCFutureTask *task, void *parameter), void *parameter)
{
    PARCFutureTask *result = parcObject_CreateInstance(PARCFutureTask);

    if (parcObject_IsInstanceOf(parameter, &PARCObject_Descriptor)) {
        parameter = parcObject_Acquire(parameter);
    }

    if (result != NULL) {
        result->function = function;
        result->parameter = parameter;
        _parcFutureTask_Initialise(result);
    }

    return result;
}

int
parcFutureTask_Compare(const PARCFutureTask *instance, const PARCFutureTask *other)
{
    int result = 0;

    return result;
}

PARCFutureTask *
parcFutureTask_Copy(const PARCFutureTask *original)
{
    PARCFutureTask *result = parcFutureTask_Create(original->function, original->parameter);

    return result;
}

void
parcFutureTask_Display(const PARCFutureTask *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCFutureTask@%p {", instance);
    /* Call Display() functions for the fields here. */
    parcDisplayIndented_PrintLine(indentation, "}");
}

bool
parcFutureTask_Equals(const PARCFutureTask *x, const PARCFutureTask *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        if (x->function == y->function) {
            if (x->parameter == y->parameter) {
                result = true;
            }
        }
    }

    return result;
}

PARCHashCode
parcFutureTask_HashCode(const PARCFutureTask *instance)
{
    PARCHashCode result = 0;

    return result;
}

bool
parcFutureTask_IsValid(const PARCFutureTask *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
parcFutureTask_ToJSON(const PARCFutureTask *instance)
{
    PARCJSON *result = parcJSON_Create();

    if (result != NULL) {
    }

    return result;
}

char *
parcFutureTask_ToString(const PARCFutureTask *instance)
{
    char *result = parcMemory_Format("PARCFutureTask@%p\n", instance);

    return result;
}

bool
parcFutureTask_Cancel(PARCFutureTask *task, bool mayInterruptIfRunning)
{
    bool result = false;

    if (parcObject_Lock(task)) {
        if (task->isRunning) {
            if (mayInterruptIfRunning) {
                printf("Interrupting a running task is not implemented yet.\n");
            }
            result = false;
        } else {
            task->isCancelled = true;
            task->isDone = true;
            parcObject_Notify(task);
            result = true;
        }

        parcObject_Unlock(task);
    }

    return result;
}

PARCFutureTaskResult
parcFutureTask_Get(const PARCFutureTask *futureTask, const PARCTimeout *timeout)
{
    PARCFutureTaskResult result;

    result.execution = PARCExecution_Timeout;
    result.value = 0;

    if (parcTimeout_IsImmediate(timeout)) {
        if (futureTask->isDone) {
            result.execution = PARCExecution_OK;
            result.value = futureTask->result;
        }
    } else {
        result.execution = PARCExecution_Interrupted;
        result.value = 0;

        parcObject_Lock(futureTask);
        while (!futureTask->isDone) {
            if (parcTimeout_IsNever(timeout)) {
                parcObject_Wait(futureTask);
                result.execution = PARCExecution_OK;
                result.value = futureTask->result;
                break;
            } else {
                if (parcObject_WaitFor(futureTask, parcTimeout_InNanoSeconds(timeout))) {
                    result.execution = PARCExecution_OK;
                    result.value = futureTask->result;
                    break;
                }
            }
        }
        parcObject_Unlock(futureTask);
    }

    return result;
}

bool
parcFutureTask_IsCancelled(const PARCFutureTask *task)
{
    return task->isCancelled;
}

bool
parcFutureTask_IsDone(const PARCFutureTask *task)
{
    return task->isDone;
}

static void *
_parcFutureTask_Execute(PARCFutureTask *task)
{
    task->isRunning = true;
    void *result = task->function(task, task->parameter);
    task->isRunning = false;

    return result;
}

void *
parcFutureTask_Run(PARCFutureTask *task)
{
    if (parcFutureTask_Lock(task)) {
        if (!task->isCancelled) {
            task->result = _parcFutureTask_Execute(task);
            task->isDone = true;
            parcFutureTask_Notify(task);
        }
        parcFutureTask_Unlock(task);
    } else {
        trapCannotObtainLock("Cannot lock PARCFutureTask");
    }
    return task->result;
}

bool
parcFutureTask_RunAndReset(PARCFutureTask *task)
{
    bool result = false;

    if (parcObject_Lock(task)) {
        if (!task->isCancelled) {
            _parcFutureTask_Execute(task);
            parcFutureTask_Reset(task);
            result = true;
        }
        parcFutureTask_Unlock(task);
    } else {
        trapCannotObtainLock("Cannot lock PARCFutureTask");
    }

    return result;
}

void
parcFutureTask_Reset(PARCFutureTask *task)
{
    _parcFutureTask_Initialise(task);
}
