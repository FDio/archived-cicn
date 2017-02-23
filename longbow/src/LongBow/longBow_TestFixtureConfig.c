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
#include <stdio.h>

#include <LongBow/longBow_TestFixtureConfig.h>
#include <LongBow/private/longBow_Memory.h>

LongBowTestFixtureConfig *
longBowTestFixtureConfig_Create(const char *name, bool enabled)
{
    LongBowTestFixtureConfig *result = longBowMemory_Allocate(sizeof(LongBowTestFixtureConfig));

    result->name = longBowMemory_StringCopy(name);
    result->enabled = enabled;

    return result;
}

void
longBowTestFixtureConfig_Destroy(LongBowTestFixtureConfig **configPtr)
{
    LongBowTestFixtureConfig *config = *configPtr;

    longBowMemory_Deallocate((void **) &config->name);
    longBowMemory_Deallocate((void **) configPtr);
}
