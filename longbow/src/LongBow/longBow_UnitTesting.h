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
 * @file longBow_UnitTesting.h
 * @ingroup testing
 * @brief Unit Testing Support.
 *
 */
#ifndef LongBow_longBow_UnitTesting_h
#define LongBow_longBow_UnitTesting_h

#include <stdbool.h>
#include <stdarg.h>

/**
 * Test a that a function implements the Equals contract.
 *
 *   The equality function that this evaluates must implement the following equivalence relations on non-null instances:
 *
 *   * It is reflexive: for any non-null reference value x, equals(x, x) must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, equals(x, y) must return true if and only if
 *        equals(y x) returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        equals(x, y) returns true and
 *        equals(y, z) returns true,
 *        then  equals(x, z) must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of equals(x, y)
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, equals(x, NULL)) must return false.
 *
 * @param [in] equalsFunction A pointer to a function that will be called to determine if it conforms to the Equals contract.
 * @param [in] x A pointer to a value that will be used as the base comparision for remaining parameters.
 * @param [in] y A pointer to a value that is known to be equal to @p x, but is not @p x.
 * @param [in] z A pointer to a value that is known to be equal to @p x and to @p y, but is neither @p x nor @p y.
 * @param [in] ... A NULL terminated list of pointers to values that are known to be not equal to @p x, @p y, or @p z.
 * @return true if the function conforms to the Equals contract.
 * @see assertEqualsContract()
 */
bool longBowUnitTesting_AssertEqualsContract(bool (*equalsFunction)(void *a, void *b), void *x, void *y, void *z, ...);

/**
 * Compares instance <code>a</code> with instance <code>b</code> for order.
 *
 * The comparison function that this evaluates <i>sgn(a - b)</i> required to return a negative integer,
 * zero, or a positive integer as <code>a</code> is less than,
 * equal to, or greater than <code>b</code>.
 *
 * The function must ensure that:
 *   <ul>
 *     <li>sgn(compareTo(a, b)) == -sgn(b, a) for all values of a and b.</li>
 *     <li>the relation is transitive: (compareTo(x, y)>0 && compareTo(y, z)>0) implies compareTo(x, z)>0.</li>
 *     <li>compareTo(x, y)== 0 implies that sgn(compareTo(x, z)) == sgn(compareTo(y, z)), for all values of z.</li>
 *   </ul>
 *
 *   This also stipulates that
 *   <code>compareTo(NULL, NULL)) == 0</code>,
 *   <code>compareTo(not-NULL, NULL)) > 0</code>,
 *   <code>compareTo(NULL, not-NULL)) < 0</code>.
 *
 * It is strongly recommended, but not strictly required that relation(compareTo(x, y)==0) == equals(x, y)) is true.
 * Any module that implements the compareTo function and violates this condition
 * should clearly indicate this fact.
 * For example, "Note: this implementation has a natural ordering that is inconsistent with equals."
 *
 * @param [in] compareTo A pointer to a function implementing the CompareTo function signature.
 * @param [in] value The pivotal value under test.
 * @param [in] equivalent A NULL terminated array of values that are all equivalent to <code>value</code>.
 * @param [in] lesser A NULL terminated array of values that are all less than <code>value</code>.
 * @param [in] greater A NULL terminated array of values that are all greater than <code>value</code>.
 * @return <code>true</code> if the evalutation is successful.
 * @see assertCompareToContract()
 */
bool longBowUnitTesting_AssertCompareToContract(int (*compareTo)(const void *a, const void *b), void *value, void **equivalent, void **lesser, void **greater);
#endif // LongBow_longBow_UnitTesting_h
