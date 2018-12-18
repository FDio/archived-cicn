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

#include <parc/algol/parc_Memory.h>
#include "parc_MyObject.h"

/*
 * Three kinds of static PARC Objects
 *
 * static or global objects defined within a module
 *
 * local objects defined within a function.
 *
 */

PARCMyObject *globalObject = parcObject_Instance(PARCMyObject, sizeof(void*), PARCMyObjectSizeOf);

void
aGlobalObject(void)
{
    int x = 1;
    int y = 2;
    double z = 3.14;

    parcObject_InitInstance(globalObject, PARCMyObject);
    parcMyObject_Init(globalObject, x, y, z);

    parcMyObject_Display(globalObject, 0);

    parcMyObject_Release(&globalObject);
}

static PARCMyObject *staticModuleObject = parcObject_Instance(PARCMyObject, sizeof(void*), PARCMyObjectSizeOf);

void
aStaticModuleObject(void)
{
    int x = 1;
    int y = 2;
    double z = 3.14;

    parcObject_InitInstance(staticModuleObject, PARCMyObject);
    parcMyObject_Init(staticModuleObject, x, y, z);

    parcMyObject_Display(staticModuleObject, 0);

    parcMyObject_Release(&staticModuleObject);
}

void
aLocalObject(void)
{
    int x = 1;
    int y = 2;
    double z = 3.14;

    PARCMyObject *localObject = parcObject_Instance(PARCMyObject, sizeof(void*), PARCMyObjectSizeOf);

    parcObject_InitInstance(localObject, PARCMyObject);
    parcMyObject_Init(localObject, x, y, z);

    parcMyObject_Display(localObject, 0);

    parcMyObject_Release(&localObject);
}

void
aWrappedObject(void)
{
    int x = 1;
    int y = 2;
    double z = 3.14;

    PARCMyObject *wrappedObject = parcMyObject_Wrap((char[parcObject_TotalSize(sizeof(void*), PARCMyObjectSizeOf)]) { 0 });
    parcMyObject_Init(wrappedObject, x, y, z);

    parcMyObject_Display(wrappedObject, 0);

    parcMyObject_Release(&wrappedObject);
}

int
main(int argc, const char *argv[argc])
{
    aGlobalObject();

    aStaticModuleObject();

    aLocalObject();

    aWrappedObject();

    return 0;
}
