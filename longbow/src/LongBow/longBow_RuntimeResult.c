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

#include <sys/time.h>

#include <LongBow/longBow_RuntimeResult.h>

size_t
longBowRuntimeResult_GetEventEvaluationCount(const LongBowRuntimeResult *testCaseResult)
{
    return testCaseResult->eventEvaluationCount;
}

LongBowEventType *
longBowRuntimeResult_GetEvent(const LongBowRuntimeResult *testCaseResult)
{
    return testCaseResult->event;
}

void
longBowRuntimeResult_SetEvent(LongBowRuntimeResult *testCaseResult, LongBowEventType *eventType)
{
    testCaseResult->event = eventType;
}

void
longBowRuntimeResult_SetStatus(LongBowRuntimeResult *testCaseResult, LongBowStatus status)
{
    testCaseResult->status = status;
}
void
longBowRuntimeResult_SetElapsedTime(LongBowRuntimeResult *testCaseResult, struct timeval *elapsedTime)
{
    testCaseResult->elapsedTime = *elapsedTime;
}

struct rusage *
longBowRuntimeResult_GetRUsage(LongBowRuntimeResult *testCaseResult)
{
    return &testCaseResult->resources;
}

void
longBowRuntimeResult_SetRUsage(LongBowRuntimeResult *testCaseResult, struct rusage *resources)
{
    testCaseResult->resources = *resources;
}

LongBowStatus
longBowRuntimeResult_GetStatus(const LongBowRuntimeResult *testCaseResult)
{
    return testCaseResult->status;
}

struct timeval
longBowRuntimeResult_GetElapsedTime(const LongBowRuntimeResult *testCaseResult)
{
    return testCaseResult->elapsedTime;
}
