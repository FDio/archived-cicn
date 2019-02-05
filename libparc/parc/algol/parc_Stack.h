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
 * @file parc_Stack.h
 * @ingroup datastructures
 * @brief PARC (Generic) Stack
 *
 * A stack interface implementation.
 *
 * Example
 * @code
 * #include <parc/algol/parc_Deque.h>
 *
 * int
 * main(int argc, char *argv[])
 * {
 *     PARCStackInterface dequeAsStack = {
 *         .parcStack_Release = parcDeque_Release,
 *         .parcStack_IsEmpty = parcDeque_IsEmpty,
 *         .parcStack_Peek = parcDeque_PeekLast,
 *         .parcStack_Pop = parcDeque_RemoveLast,
 *         .parcStack_Push = parcDeque_Append,
 *         .parcStack_Search = NULL
 *     };
 *
 *     PARCStack *stack = parcStack(parcDeque_Create(), &dequeAsStack);
 *
 *     parcStack_IsEmpty(stack);
 * }
 * @endcode
 *
 *
 */
#ifndef libparc_parc_Stack_h
#define libparc_parc_Stack_h

#include <stdbool.h>

struct parc_stack;
typedef struct parc_stack PARCStack;

typedef struct parc_stack_interface {
    /**
     * Release the instance
     *
     *
     * @param [in,out] instancePtr A pointer to the pointer of the instance to release.
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    void (*parcStack_Release)(void **instancePtr);

    /**
     * Tests if this stack is empty.
     *
     * @param [in] instance A pointer to the instance to test.
     * @return true if the stack is empty
     * @return false if the stack is not empty
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    bool (*parcStack_IsEmpty)(const void *instance);

    /**
     * Looks at the object at the top of this stack without removing it from the stack.
     *
     * @param [in] instance A pointer to the instance to look at.
     * @return The object at the top of the @p instance.
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    void * (*parcStack_Peek)(const void *instance);

    /**
     * Removes the object at the top of this stack and returns that object as the value of this function.
     *
     *
     * @param [in,out] instance A pointer to the instance to check and modify.
     * @return The object at the top of the @p instance.
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    void * (*parcStack_Pop)(void *instance);

    /**
     * Pushes an item onto the top of this stack.
     *
     *
     * @param [in,out] instance A pointer to the instance to modify.
     * @param [in] item A pointer to the object to push on the @p instance.
     * @return A pointer to the object that was pushed on the @p instance.
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    void * (*parcStack_Push)(void *instance, void *item);

    /**
     * Returns the 1-based position where an object is on this stack.
     *
     *
     * @param [in] instance A pointer to the instance.
     * @param [in] element A pointer to the element to find on the @p instance.
     * @return The index of the position where @p element is found in the @p instance.
     *
     * Example:
     * @code
     * <#example#>
     * @endcode
     */
    int (*parcStack_Search)(void *instance, void *element);
} PARCStackInterface;
#endif // libparc_parc_Stack_h
