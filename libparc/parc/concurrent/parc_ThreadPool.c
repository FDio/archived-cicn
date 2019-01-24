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

#include <parc/concurrent/parc_AtomicUint64.h>
#include <parc/concurrent/parc_ThreadPool.h>
#include <parc/concurrent/parc_Thread.h>

struct PARCThreadPool {
    bool continueExistingPeriodicTasksAfterShutdown;
    bool executeExistingDelayedTasksAfterShutdown;
    bool removeOnCancel;
    PARCLinkedList *workQueue;
    PARCLinkedList *threads;
    int poolSize;
    int maximumPoolSize;
    long taskCount;
    bool isShutdown;
    bool isTerminated;
    bool isTerminating;

    PARCAtomicUint64 *completedTaskCount;
};

static void *
_parcThreadPool_Worker(const PARCThread *thread, const PARCThreadPool *pool)
{
    while (parcThread_IsCancelled(thread) == false && pool->isTerminated == false) {
        if (parcLinkedList_Lock(pool->workQueue)) {
            PARCFutureTask *task = parcLinkedList_RemoveFirst(pool->workQueue);
            if (task != NULL) {
                parcAtomicUint64_Increment(pool->completedTaskCount);
                parcLinkedList_Unlock(pool->workQueue);
                parcFutureTask_Run(task);
                parcFutureTask_Release(&task);
                parcLinkedList_Lock(pool->workQueue);

                parcLinkedList_Notify(pool->workQueue);
            } else {
                parcLinkedList_WaitFor(pool->workQueue, 1000000000);
            }
            parcLinkedList_Unlock(pool->workQueue);
        }
    }

    return NULL;
}

static void
_parcThreadPool_CancelAll(const PARCThreadPool *pool)
{
    PARCIterator *iterator = parcLinkedList_CreateIterator(pool->threads);

    while (parcIterator_HasNext(iterator)) {
        PARCThread *thread = parcIterator_Next(iterator);
        parcThread_Cancel(thread);
    }
    parcIterator_Release(&iterator);
}

static void
_parcThreadPool_JoinAll(const PARCThreadPool *pool)
{
    PARCIterator *iterator = parcLinkedList_CreateIterator(pool->threads);

    while (parcIterator_HasNext(iterator)) {
        PARCThread *thread = parcIterator_Next(iterator);
        parcThread_Join(thread);
    }
    parcIterator_Release(&iterator);
}

static bool
_parcThreadPool_Destructor(PARCThreadPool **instancePtr)
{
    parcAssertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCThreadPool pointer.");
    PARCThreadPool *pool = *instancePtr;

    if (pool->isShutdown == false) {
        _parcThreadPool_CancelAll(pool);
        _parcThreadPool_JoinAll(pool);
    }

    parcAtomicUint64_Release(&pool->completedTaskCount);
    parcLinkedList_Release(&pool->threads);

    if (parcObject_Lock(pool->workQueue)) {
        parcLinkedList_Release(&pool->workQueue);
    }

    return true;
}

parcObject_ImplementAcquire(parcThreadPool, PARCThreadPool);

parcObject_ImplementRelease(parcThreadPool, PARCThreadPool);

parcObject_Override(PARCThreadPool, PARCObject,
                    .isLockable = true,
                    .destructor = (PARCObjectDestructor *) _parcThreadPool_Destructor,
                    .copy = (PARCObjectCopy *) parcThreadPool_Copy,
                    .toString = (PARCObjectToString *) parcThreadPool_ToString,
                    .equals = (PARCObjectEquals *) parcThreadPool_Equals,
                    .compare = (PARCObjectCompare *) parcThreadPool_Compare,
                    .hashCode = (PARCObjectHashCode *) parcThreadPool_HashCode);

void
parcThreadPool_AssertValid(const PARCThreadPool *instance)
{
    parcAssertTrue(parcThreadPool_IsValid(instance),
               "PARCThreadPool is not valid.");
}


PARCThreadPool *
parcThreadPool_Create(int poolSize)
{
    PARCThreadPool *result = parcObject_CreateInstance(PARCThreadPool);

    if (result != NULL) {
        result->poolSize = poolSize;
        result->maximumPoolSize = poolSize;
        result->taskCount = 0;
        result->isShutdown = false;
        result->isTerminated = false;
        result->isTerminating = false;
        result->workQueue = parcLinkedList_Create();
        result->threads = parcLinkedList_Create();

        result->completedTaskCount = parcAtomicUint64_Create(0);

        result->continueExistingPeriodicTasksAfterShutdown = false;
        result->executeExistingDelayedTasksAfterShutdown = false;
        result->removeOnCancel = true;

        if (parcObject_Lock(result)) {
            for (int i = 0; i < poolSize; i++) {
                PARCThread *thread = parcThread_Create((void *(*)(PARCThread *, PARCObject *))_parcThreadPool_Worker, (PARCObject *) result);
                parcLinkedList_Append(result->threads, thread);
                parcThread_Start(thread);
                parcThread_Release(&thread);
            }
            parcObject_Unlock(result);
        }
    }

    return result;
}

int
parcThreadPool_Compare(const PARCThreadPool *instance, const PARCThreadPool *other)
{
    int result = 0;

    return result;
}

PARCThreadPool *
parcThreadPool_Copy(const PARCThreadPool *original)
{
    PARCThreadPool *result = parcThreadPool_Create(original->poolSize);

    return result;
}

void
parcThreadPool_Display(const PARCThreadPool *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCThreadPool@%p {", instance);
    /* Call Display() functions for the fields here. */
    parcDisplayIndented_PrintLine(indentation, "}");
}

bool
parcThreadPool_Equals(const PARCThreadPool *x, const PARCThreadPool *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        /* perform instance specific equality tests here. */
        if (x->poolSize == y->poolSize) {
            result = true;
        }
    }

    return result;
}

PARCHashCode
parcThreadPool_HashCode(const PARCThreadPool *instance)
{
    PARCHashCode result = 0;

    return result;
}

bool
parcThreadPool_IsValid(const PARCThreadPool *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
parcThreadPool_ToJSON(const PARCThreadPool *instance)
{
    PARCJSON *result = parcJSON_Create();

    if (result != NULL) {
    }

    return result;
}

char *
parcThreadPool_ToString(const PARCThreadPool *instance)
{
    char *result = parcMemory_Format("PARCThreadPool@%p\n", instance);

    return result;
}

void
parcThreadPool_SetAllowCoreThreadTimeOut(PARCThreadPool *pool, bool value)
{
}

bool
parcThreadPool_GetAllowsCoreThreadTimeOut(const PARCThreadPool *pool)
{
    return false;
}

bool
parcThreadPool_AwaitTermination(PARCThreadPool *pool, PARCTimeout *timeout)
{
    bool result = false;

    if (pool->isTerminating) {
        if (parcLinkedList_Lock(pool->workQueue)) {
            while (parcLinkedList_Size(pool->workQueue) > 0) {
                if (parcTimeout_IsNever(timeout)) {
                    parcLinkedList_Wait(pool->workQueue);
                } else {
                    // This is not accurate as this will continue the delay, rather than keep a cumulative amount of delay.
                    uint64_t delay = parcTimeout_InNanoSeconds(timeout);
                    parcLinkedList_WaitFor(pool->workQueue, delay);
                }
            }
            result = true;
            parcLinkedList_Unlock(pool->workQueue);
        }

        parcThreadPool_ShutdownNow(pool);
    }

    return result;
}

bool
parcThreadPool_Execute(PARCThreadPool *pool, PARCFutureTask *task)
{
    bool result = false;

    if (parcThreadPool_Lock(pool)) {
        if (pool->isShutdown == false) {
            parcThreadPool_Unlock(pool);
            if (parcLinkedList_Lock(pool->workQueue)) {
                parcLinkedList_Append(pool->workQueue, task);
                parcLinkedList_Notify(pool->workQueue);
                parcLinkedList_Unlock(pool->workQueue);
                result = true;
            }
        } else {
            parcThreadPool_Unlock(pool);
        }
    }

    return result;
}

int
parcThreadPool_GetActiveCount(const PARCThreadPool *pool)
{
    return pool->poolSize;
}

uint64_t
parcThreadPool_GetCompletedTaskCount(const PARCThreadPool *pool)
{
    return parcAtomicUint64_GetValue(pool->completedTaskCount);
}

int
parcThreadPool_GetCorePoolSize(const PARCThreadPool *pool)
{
    return pool->poolSize;
}

PARCTimeout *
parcThreadPool_GetKeepAliveTime(const PARCThreadPool *pool)
{
    return PARCTimeout_Never;
}

int
parcThreadPool_GetLargestPoolSize(const PARCThreadPool *pool)
{
    return pool->poolSize;
}

int
parcThreadPool_GetMaximumPoolSize(const PARCThreadPool *pool)
{
    return pool->maximumPoolSize;
}

int
parcThreadPool_GetPoolSize(const PARCThreadPool *pool)
{
    return pool->poolSize;
}

PARCLinkedList *
parcThreadPool_GetQueue(const PARCThreadPool *pool)
{
    return pool->workQueue;
}

long
parcThreadPool_GetTaskCount(const PARCThreadPool *pool)
{
    return pool->taskCount;
}

bool
parcThreadPool_IsShutdown(const PARCThreadPool *pool)
{
    return pool->isShutdown;
}

bool
parcThreadPool_IsTerminated(const PARCThreadPool *pool)
{
    return pool->isTerminated;
}

bool
parcThreadPool_IsTerminating(const PARCThreadPool *pool)
{
    return pool->isTerminating;
}

int
parcThreadPool_PrestartAllCoreThreads(PARCThreadPool *pool)
{
    return 0;
}

bool
parcThreadPool_PrestartCoreThread(PARCThreadPool *pool)
{
    return 0;
}

void
parcThreadPool_Purge(PARCThreadPool *pool)
{
}

bool
parcThreadPool_Remove(PARCThreadPool *pool, PARCFutureTask *task)
{
    return false;
}

void
parcThreadPool_SetCorePoolSize(PARCThreadPool *pool, int corePoolSize)
{
}

void
parcThreadPool_SetKeepAliveTime(PARCThreadPool *pool, PARCTimeout *timeout)
{
}

void
parcThreadPool_SetMaximumPoolSize(PARCThreadPool *pool, int maximumPoolSize)
{
}

void
parcThreadPool_Shutdown(PARCThreadPool *pool)
{
    if (parcThreadPool_Lock(pool)) {
        pool->isShutdown = true;
        pool->isTerminating = true;
        parcThreadPool_Unlock(pool);
    }
}

PARCLinkedList *
parcThreadPool_ShutdownNow(PARCThreadPool *pool)
{
    parcThreadPool_Shutdown(pool);

    // Cause all of the worker threads to exit.
    _parcThreadPool_CancelAll(pool);

    // Wake them all up so they detect that they are cancelled.
    if (parcThreadPool_Lock(pool)) {
        parcThreadPool_NotifyAll(pool);
        parcThreadPool_Unlock(pool);
    }

    if (parcLinkedList_Lock(pool->workQueue)) {
        parcLinkedList_NotifyAll(pool->workQueue);
        parcLinkedList_Unlock(pool->workQueue);
    }
    // Join with all of them, thereby cleaning up all of them.
    _parcThreadPool_JoinAll(pool);

    pool->isTerminated = true;
    return NULL;
}
