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

#include <stdio.h>
#include "../runtime.h"

# define longBowAssert(eventPointer, condition, ...) \
    longBowRuntime_EventEvaluation(eventPointer); \
    if (longBowIsFalse(condition) && \
        (longBowRuntime_EventTrigger(eventPointer, longBowLocation_Create(__FILE__, longBow_function, __LINE__), #condition, __VA_ARGS__), true)) \
        for (; true; abort())


int
main(int argc, char *argv[argc])
{
    int condition = 1;

    longBowAssert(&LongBowAssertEvent, condition == 1, "Message %d", 2)
    {
        printf("Should not have Triggered\n");
    };
    longBowAssert(&LongBowAssertEvent, condition == 0, "Message %d", 2)
    {
        printf("Triggered\n");
    };

    longBowAssert(&LongBowAssertEvent, condition == 0, "Message %d", 2);
}
