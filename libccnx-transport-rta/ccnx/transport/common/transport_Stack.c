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

#include <config.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>

#include <transport_Stack.h>

struct TransportStack {
    int topFD;
    int bottomFD;
};

static void
_transportStack_Finalize(TransportStack **instancePtr)
{
    assertNotNull(instancePtr, "Parameter must be a non-null pointer to a TransportStack pointer.");
    TransportStack *instance = *instancePtr;

    transportStack_OptionalAssertValid(instance);

    /* cleanup the instance fields here */
}

parcObject_ImplementAcquire(transportStack, TransportStack);

parcObject_ImplementRelease(transportStack, TransportStack);

parcObject_ExtendPARCObject(TransportStack, _transportStack_Finalize, transportStack_Copy, transportStack_ToString, transportStack_Equals, transportStack_Compare, transportStack_HashCode, transportStack_ToJSON);


void
transportStack_AssertValid(const TransportStack *instance)
{
    assertTrue(transportStack_IsValid(instance),
               "TransportStack is not valid.");
}


TransportStack *
transportStack_Create(void)
{
    TransportStack *result = parcObject_CreateInstance(TransportStack);

    return result;
}

int
transportStack_Compare(const TransportStack *instance, const TransportStack *other)
{
    int result = 0;

    return result;
}

TransportStack *
transportStack_Copy(const TransportStack *original)
{
    TransportStack *result = NULL;

    return result;
}

void
transportStack_Display(const TransportStack *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "TransportStack@%p {", instance);
    /* Call Display() functions for the fields here. */
    parcDisplayIndented_PrintLine(indentation, "}");
}

bool
transportStack_Equals(const TransportStack *x, const TransportStack *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        /* perform instance specific equality tests here. */
    }

    return result;
}

PARCHashCode
transportStack_HashCode(const TransportStack *instance)
{
    PARCHashCode result = 0;

    return result;
}

bool
transportStack_IsValid(const TransportStack *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
transportStack_ToJSON(const TransportStack *instance)
{
    PARCJSON *result = parcJSON_Create();

    return result;
}

char *
transportStack_ToString(const TransportStack *instance)
{
    char *result = parcMemory_Format("TransportStack@%p\n", instance);

    return result;
}
