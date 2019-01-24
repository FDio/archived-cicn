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

#include <parc/assert/parc_Assert.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>

#include <parc/logging/parc_LogManager.h>

struct PARCLogManager {
    void *thisIsntFinishedYet;
};

#if 0
static void
_parcLogManager_Destroy(PARCLogManager **instancePtr)
{
    //PARCLogManager *instance = *instancePtr;
}
#endif

PARCLogManager *
parcLogManager_Create(void)
{
    return NULL;
}

PARCLogManager *
parcLogManager_Acquire(const PARCLogManager *instance)
{
    return parcObject_Acquire(instance);
}

void
parcLogManager_Release(PARCLogManager **instancePtr)
{
    parcObject_Release((void **) instancePtr);
}
