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
#include <parc/algol/parc_SortedList.h>
#include <parc/algol/parc_LinkedList.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Time.h>

#include <parc/concurrent/parc_ScheduledThreadPool.h>
#include <parc/concurrent/parc_Thread.h>
#include <parc/concurrent/parc_ThreadPool.h>

struct PARCScheduledThreadPool {
    bool continueExistingPeriodicTasksAfterShutdown;
    bool executeExistingDelayedTasksAfterShutdown;
    bool removeOnCancel;
    PARCSortedList *workQueue;
    PARCThread *workerThread;
    PARCThreadPool *threadPool;
    int poolSize;
};

static void *
_workerThread(PARCThread *thread, PARCScheduledThreadPool *pool)
{
    while (parcThread_IsCancelled(thread) == false) {
        if (parcSortedList_Lock(pool->workQueue)) {
            if (parcSortedList_Size(pool->workQueue) > 0) {
                PARCScheduledTask *task = parcSortedList_GetFirst(pool->workQueue);
                int64_t executionDelay = parcScheduledTask_GetExecutionTime(task) - parcTime_NowNanoseconds();
                if (task != NULL && executionDelay <= 0) {
                    parcSortedList_RemoveFirst(pool->workQueue);
                    parcSortedList_Unlock(pool->workQueue);
                    parcThreadPool_Execute(pool->threadPool, parcScheduledTask_GetTask(task));
                    parcScheduledTask_Release(&task);
                    parcSortedList_Lock(pool->workQueue);

                    parcSortedList_Notify(pool->workQueue);
                } else {
                    parcSortedList_WaitFor(pool->workQueue, executionDelay);
                }
            } else {
                parcSortedList_Wait(pool->workQueue);
            }
        }
        parcSortedList_Unlock(pool->workQueue);
    }

    return NULL;
}

static bool
_parcScheduledThreadPool_Destructor(PARCScheduledThreadPool **instancePtr)
{
    assertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCScheduledThreadPool pointer.");
    PARCScheduledThreadPool *pool = *instancePtr;
    parcThreadPool_Release(&pool->threadPool);

    parcThread_Release(&pool->workerThread);

    if (parcObject_Lock(pool->workQueue)) {
        parcSortedList_Release(&pool->workQueue);
    } else {
        assertTrue(false, "Cannot lock the work queue.");
    }

    return true;
}

parcObject_ImplementAcquire(parcScheduledThreadPool, PARCScheduledThreadPool);

parcObject_ImplementRelease(parcScheduledThreadPool, PARCScheduledThreadPool);

parcObject_Override(PARCScheduledThreadPool, PARCObject,
                    .isLockable = true,
                    .destructor = (PARCObjectDestructor *) _parcScheduledThreadPool_Destructor,
                    .copy = (PARCObjectCopy *) parcScheduledThreadPool_Copy,
                    .toString = (PARCObjectToString *) parcScheduledThreadPool_ToString,
                    .equals = (PARCObjectEquals *) parcScheduledThreadPool_Equals,
                    .compare = (PARCObjectCompare *) parcScheduledThreadPool_Compare,
                    .hashCode = (PARCObjectHashCode *) parcScheduledThreadPool_HashCode);

void
parcScheduledThreadPool_AssertValid(const PARCScheduledThreadPool *instance)
{
    assertTrue(parcScheduledThreadPool_IsValid(instance),
               "PARCScheduledThreadPool is not valid.");
}

PARCScheduledThreadPool *
parcScheduledThreadPool_Create(int poolSize)
{
    PARCScheduledThreadPool *result = parcObject_CreateInstance(PARCScheduledThreadPool);

    if (result != NULL) {
        result->poolSize = poolSize;
        result->workQueue = parcSortedList_Create();
        result->threadPool = parcThreadPool_Create(poolSize);

        result->continueExistingPeriodicTasksAfterShutdown = false;
        result->executeExistingDelayedTasksAfterShutdown = false;
        result->removeOnCancel = true;

        if (parcObject_Lock(result)) {
            result->workerThread = parcThread_Create((void *(*)(PARCThread *, PARCObject *))_workerThread, (PARCObject *) result);
            parcThread_Start(result->workerThread);
            parcObject_Unlock(result);
        }
    }

    return result;
}

int
parcScheduledThreadPool_Compare(const PARCScheduledThreadPool *instance, const PARCScheduledThreadPool *other)
{
    int result = 0;

    return result;
}

PARCScheduledThreadPool *
parcScheduledThreadPool_Copy(const PARCScheduledThreadPool *original)
{
    PARCScheduledThreadPool *result = parcScheduledThreadPool_Create(original->poolSize);

    return result;
}

void
parcScheduledThreadPool_Display(const PARCScheduledThreadPool *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCScheduledThreadPool@%p {", instance);
    /* Call Display() functions for the fields here. */
    parcDisplayIndented_PrintLine(indentation, "}");
}

bool
parcScheduledThreadPool_Equals(const PARCScheduledThreadPool *x, const PARCScheduledThreadPool *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        if (x->poolSize == y->poolSize) {
            result = true;
        }
    }

    return result;
}

PARCHashCode
parcScheduledThreadPool_HashCode(const PARCScheduledThreadPool *instance)
{
    PARCHashCode result = 0;

    return result;
}

bool
parcScheduledThreadPool_IsValid(const PARCScheduledThreadPool *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
parcScheduledThreadPool_ToJSON(const PARCScheduledThreadPool *instance)
{
    PARCJSON *result = parcJSON_Create();

    if (result != NULL) {
    }

    return result;
}

char *
parcScheduledThreadPool_ToString(const PARCScheduledThreadPool *instance)
{
    char *result = parcMemory_Format("PARCScheduledThreadPool@%p\n", instance);

    return result;
}

void
parcScheduledThreadPool_Execute(PARCScheduledThreadPool *pool, PARCFutureTask *command)
{
}

bool
parcScheduledThreadPool_GetContinueExistingPeriodicTasksAfterShutdownPolicy(PARCScheduledThreadPool *pool)
{
    return pool->continueExistingPeriodicTasksAfterShutdown;
}

bool
parcScheduledThreadPool_GetExecuteExistingDelayedTasksAfterShutdownPolicy(PARCScheduledThreadPool *pool)
{
    return pool->executeExistingDelayedTasksAfterShutdown;
}

PARCSortedList *
parcScheduledThreadPool_GetQueue(const PARCScheduledThreadPool *pool)
{
    return pool->workQueue;
}

bool
parcScheduledThreadPool_GetRemoveOnCancelPolicy(const PARCScheduledThreadPool *pool)
{
    return pool->removeOnCancel;
}

PARCScheduledTask *
parcScheduledThreadPool_Schedule(PARCScheduledThreadPool *pool, PARCFutureTask *task, const PARCTimeout *delay)
{
    uint64_t executionTime = parcTime_NowNanoseconds() + parcTimeout_InNanoSeconds(delay);

    PARCScheduledTask *scheduledTask = parcScheduledTask_Create(task, executionTime);

    if (parcSortedList_Lock(pool->workQueue)) {
        parcSortedList_Add(pool->workQueue, scheduledTask);
        parcScheduledTask_Release(&scheduledTask);
        parcSortedList_Notify(pool->workQueue);
        parcSortedList_Unlock(pool->workQueue);
    }
    return scheduledTask;
}

PARCScheduledTask *
parcScheduledThreadPool_ScheduleAtFixedRate(PARCScheduledThreadPool *pool, PARCFutureTask *task, PARCTimeout initialDelay, PARCTimeout period)
{
    return NULL;
}

PARCScheduledTask *
parcScheduledThreadPool_ScheduleWithFixedDelay(PARCScheduledThreadPool *pool, PARCFutureTask *task, PARCTimeout initialDelay, PARCTimeout delay)
{
    return NULL;
}

void
parcScheduledThreadPool_SetContinueExistingPeriodicTasksAfterShutdownPolicy(PARCScheduledThreadPool *pool, bool value)
{
}

void
parcScheduledThreadPool_SetExecuteExistingDelayedTasksAfterShutdownPolicy(PARCScheduledThreadPool *pool, bool value)
{
}

void
parcScheduledThreadPool_SetRemoveOnCancelPolicy(PARCScheduledThreadPool *pool, bool value)
{
}

void
parcScheduledThreadPool_Shutdown(PARCScheduledThreadPool *pool)
{
    parcScheduledThreadPool_ShutdownNow(pool);
}

PARCList *
parcScheduledThreadPool_ShutdownNow(PARCScheduledThreadPool *pool)
{
    parcThread_Cancel(pool->workerThread);

    parcThreadPool_ShutdownNow(pool->threadPool);

    // Wake them all up so they detect that they are cancelled.
    if (parcObject_Lock(pool)) {
        parcObject_NotifyAll(pool);
        parcObject_Unlock(pool);
    }
    if (parcObject_Lock(pool->workQueue)) {
        parcObject_NotifyAll(pool->workQueue);
        parcObject_Unlock(pool->workQueue);
    }

    parcThread_Join(pool->workerThread);

    return NULL;
}

PARCScheduledTask *
parcScheduledThreadPool_Submit(PARCScheduledThreadPool *pool, PARCFutureTask *task)
{
    PARCScheduledTask *scheduledTask = parcScheduledTask_Create(task, 0);

    parcSortedList_Add(pool->workQueue, scheduledTask);

    return scheduledTask;
}
