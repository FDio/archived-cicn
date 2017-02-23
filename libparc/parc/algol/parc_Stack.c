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

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Deque.h>
#include <parc/algol/parc_Stack.h>

struct parc_stack {
    void *instance;
    PARCStackInterface *interface;
};

PARCStack *
parcStack(void *instance, PARCStackInterface *interface)
{
    PARCStack *result = parcMemory_AllocateAndClear(sizeof(PARCStack));
    assertNotNull(result, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(PARCStack));
    result->instance = instance;
    result->interface = interface;

    return result;
}

void
parcStack_Release(PARCStack **stackPtr)
{
    PARCStack *stack = *stackPtr;
    (stack->interface->parcStack_Release)(&stack->instance);
    parcMemory_Deallocate((void **) &stack);

    *stackPtr = 0;
}

bool
parcStack_IsEmpty(const PARCStack *stack)
{
    return (stack->interface->parcStack_IsEmpty)(stack->instance);
}

void *
parcStack_Peek(const PARCStack *stack)
{
    return (stack->interface->parcStack_Peek)(stack->instance);
}

void *
parcStack_Pop(PARCStack *stack)
{
    return (stack->interface->parcStack_Pop)(stack->instance);
}

void *
parcStack_Push(PARCStack *stack, void *element)
{
    return (stack->interface->parcStack_Push)(stack->instance, element);
}
