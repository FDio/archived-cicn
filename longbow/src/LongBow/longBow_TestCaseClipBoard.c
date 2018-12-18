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

#include <stdlib.h>

#include <LongBow/longBow_TestCaseClipBoard.h>
#include <LongBow/private/longBow_Memory.h>

struct longbow_testcase_clipboard {
    /**
     * A pointer to arbitrary data shared between the setup, test case, and teardown.
     */
    void *shared;
};

LongBowTestCaseClipBoard *
longBowTestCaseClipBoard_Create(void *shared)
{
    LongBowTestCaseClipBoard *result = longBowMemory_Allocate(sizeof(LongBowTestCaseClipBoard));
    longBowTestCaseClipBoard_Set(result, shared);
    return result;
}

void
longBowTestCaseClipBoard_Destroy(LongBowTestCaseClipBoard **clipBoardPtr)
{
    longBowMemory_Deallocate((void **) clipBoardPtr);
}

void *
longBowTestCaseClipBoard_Get(const LongBowTestCaseClipBoard *clipBoard)
{
    return clipBoard->shared;
}

LongBowTestCaseClipBoard *
longBowTestCaseClipBoard_Set(LongBowTestCaseClipBoard *clipBoard, void *shared)
{
    void *previousValue = clipBoard->shared;
    clipBoard->shared = shared;
    return previousValue;
}
