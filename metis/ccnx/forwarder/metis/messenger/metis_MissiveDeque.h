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
 * @file metis_MissiveDeque
 * @brief Double ended queue of Missives
 *
 * Used to queue Missives.  This is a type-safe wrapper around {@link PARCDeque}
 *
 */

#ifndef Metis_metis_MissiveDeque_h
#define Metis_metis_MissiveDeque_h

struct metis_missive_deque;

typedef struct metis_missive_deque MetisMissiveDeque;

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
MetisMissiveDeque *metisMissiveDeque_Create(void);

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
void metisMissiveDeque_Release(MetisMissiveDeque **dequePtr);

/**
 * Appends the missive to the queue, taking ownership of the memory
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
MetisMissiveDeque *metisMissiveDeque_Append(MetisMissiveDeque *deque, MetisMissive *missive);

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
MetisMissive *metisMissiveDeque_RemoveFirst(MetisMissiveDeque *deque);

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
size_t metisMissiveDeque_Size(const MetisMissiveDeque *deque);
#endif // Metis_metis_MissiveDeque_h
