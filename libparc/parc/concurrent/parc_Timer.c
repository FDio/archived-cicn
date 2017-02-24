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
#include <parc/algol/parc_Memory.h>

#include "parc_Timer.h"

struct PARCTimer {
    int delay;
};

static void
_parcTimer_Finalize(PARCTimer **instancePtr)
{
    assertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCTimer pointer.");


    /* cleanup the instance fields here */
}

parcObject_ImplementAcquire(parcTimer, PARCTimer);

parcObject_ImplementRelease(parcTimer, PARCTimer);

parcObject_ExtendPARCObject(PARCTimer, _parcTimer_Finalize, parcTimer_Copy, parcTimer_ToString, parcTimer_Equals, parcTimer_Compare, parcTimer_HashCode, parcTimer_ToJSON);


void
parcTimer_AssertValid(const PARCTimer *instance)
{
    assertTrue(parcTimer_IsValid(instance),
               "PARCTimer is not valid.");
}

PARCTimer *
parcTimer_Create(void)
{
    PARCTimer *result = parcObject_CreateInstance(PARCTimer);

    if (result != NULL) {
    }

    return result;
}

int
parcTimer_Compare(const PARCTimer *instance, const PARCTimer *other)
{
    int result = 0;

    return result;
}

PARCTimer *
parcTimer_Copy(const PARCTimer *original)
{
    PARCTimer *result = parcTimer_Create();

    return result;
}

void
parcTimer_Display(const PARCTimer *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCTimer@%p {", instance);
    /* Call Display() functions for the fields here. */
    parcDisplayIndented_PrintLine(indentation, "}");
}

bool
parcTimer_Equals(const PARCTimer *x, const PARCTimer *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        return true;
    }

    return result;
}

PARCHashCode
parcTimer_HashCode(const PARCTimer *instance)
{
    PARCHashCode result = 0;

    return result;
}

bool
parcTimer_IsValid(const PARCTimer *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
parcTimer_ToJSON(const PARCTimer *instance)
{
    PARCJSON *result = parcJSON_Create();

    if (result != NULL) {
    }

    return result;
}

char *
parcTimer_ToString(const PARCTimer *instance)
{
    char *result = parcMemory_Format("PARCTimer@%p\n", instance);

    return result;
}

void
parcTimer_Cancel(PARCTimer *timer)
{
}

int
parcTimer_Purge(PARCTimer *timer)
{
    return 0;
}

void
parcTimer_ScheduleAtTime(PARCTimer *timer, PARCFutureTask *task, time_t absoluteTime)
{
}

void
parcTimer_ScheduleAtTimeAndRepeat(PARCTimer *timer, PARCFutureTask *task, time_t firstTime, long period)
{
}

void
parcTimer_ScheduleAfterDelay(PARCTimer *timer, PARCFutureTask *task, long delay)
{
}

void
parcTimer_ScheduleAfterDelayAndRepeat(PARCTimer *timer, PARCFutureTask *task, long delay, long period)
{
}

