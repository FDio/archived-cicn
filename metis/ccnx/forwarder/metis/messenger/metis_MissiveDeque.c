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
 * A type-safe wrapper for Missives around a {@link PARCDeque}.  We only implement
 * the subset of functions used.
 *
 */

#include <config.h>
#include <stdio.h>
#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Deque.h>

#include <ccnx/forwarder/metis/messenger/metis_Missive.h>
#include <ccnx/forwarder/metis/messenger/metis_MissiveDeque.h>

struct metis_missive_deque {
    PARCDeque *queue;
};

/**
 * Create a `PARCDeque` instance with the default element equals function.
 *
 * The queue is created with no elements.
 *
 * The default element equals function is used by the `parcDeque_Equals` function and
 * simply compares the values using the `==` operator.
 * Users that need more sophisticated comparisons of the elements need to supply their own
 * function via the `parcDeque_CreateCustom` function.
 *
 * @return non-NULL A pointer to a PARCDeque instance.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
MetisMissiveDeque *
metisMissiveDeque_Create(void)
{
    MetisMissiveDeque *missiveDeque = parcMemory_AllocateAndClear(sizeof(MetisMissiveDeque));
    assertNotNull(missiveDeque, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisMissiveDeque));
    missiveDeque->queue = parcDeque_Create();
    return missiveDeque;
}

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
void
metisMissiveDeque_Release(MetisMissiveDeque **dequePtr)
{
    assertNotNull(dequePtr, "Double pointer must be non-null");
    assertNotNull(*dequePtr, "Double pointer must dereference to non-null");
    MetisMissiveDeque *missiveDeque = *dequePtr;

    // flush the queue
    while (!parcDeque_IsEmpty(missiveDeque->queue)) {
        MetisMissive *missive = metisMissiveDeque_RemoveFirst(missiveDeque);
        metisMissive_Release(&missive);
    }

    parcDeque_Release(&missiveDeque->queue);
    parcMemory_Deallocate((void **) &missiveDeque);
    *dequePtr = NULL;
}

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
MetisMissiveDeque *
metisMissiveDeque_Append(MetisMissiveDeque *deque, MetisMissive *missive)
{
    assertNotNull(deque, "Parameter deque must be non-null");
    assertNotNull(missive, "Parameter missive must be non-null");

    parcDeque_Append(deque->queue, missive);
    return deque;
}

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
MetisMissive *
metisMissiveDeque_RemoveFirst(MetisMissiveDeque *deque)
{
    assertNotNull(deque, "Parameter deque must be non-null");
    return (MetisMissive *) parcDeque_RemoveFirst(deque->queue);
}

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see <#references#>
 */
size_t
metisMissiveDeque_Size(const MetisMissiveDeque *deque)
{
    assertNotNull(deque, "Parameter deque must be non-null");
    return parcDeque_Size(deque->queue);
}
