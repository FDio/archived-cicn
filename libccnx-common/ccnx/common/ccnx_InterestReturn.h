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
 * @file ccnx_InterestReturn.h
 * @ingroup InterestRetrun
 * @brief A CCNx InterestReturn is an optional error response for an Interest that can't be satisfied by the returning entity
 *
 * An InterestReturn is a convience type for a returned Interest. It is created from an CCNxInterest and a return code
 * with the intent of returning the interest to the previous hop. Other than modifing the PacketType to indicate that it
 * is a Interest Return, it wraps and preserves the state of the provided CCNxInterest and can be used with CCNxInterest
 * functions as if it were a CCNxInterest type.
 *
 * The possible return codes are:
 *
 *     +--------------------+
 *     | No Route           |
 *     |                    |
 *     | Hop Limit Exceeded |
 *     |                    |
 *     | No Resources       |
 *     |                    |
 *     | Path Error         |
 *     |                    |
 *     | Prohibited         |
 *     |                    |
 *     | Congested          |
 *     |                    |
 *     | MTU too large      |
 *     +--------------------+
 *
 * @see {@link CCNxInterest}
 *
 */

#ifndef libccnx_ccnx_InterestReturn_h
#define libccnx_ccnx_InterestReturn_h

#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <ccnx/common/ccnx_Interest.h>

/**
 * @typedef CCNxInterestReturn
 * @brief The CCNx InterestReturn Message
 */
typedef CCNxTlvDictionary CCNxInterestReturn;

/**
 * @typedef CCNxInterestReturn_ReturnCode
 * @brief The CCNx InterestReturn ReturnCode options
 */
typedef enum {
    CCNxInterestReturn_ReturnCode_NoRoute = 1,
    CCNxInterestReturn_ReturnCode_HopLimitExceeded = 2,
    CCNxInterestReturn_ReturnCode_NoResources = 3,
    CCNxInterestReturn_ReturnCode_PathError = 4,
    CCNxInterestReturn_ReturnCode_Prohibited = 5,
    CCNxInterestReturn_ReturnCode_Congestion = 6,
    CCNxInterestReturn_ReturnCode_MTUTooLarge = 7,
    CCNxInterestReturn_ReturnCode_END = 8
} CCNxInterestReturn_ReturnCode;

/**
 * Create a new instance of `CCNxInterestRetrun` from the specified CCNxInterest, with the specified return code.
 *
 * The created instance of `CCNxInterestReturn` must be released by calling {@link ccnxInterestReturn_Release}().
 *
 * @param [in] interest A pointer to the {@link CCNxInterest} to be returned.
 * @param [in] returnCode The {@link CCNxInterestReturn_ReturnCode} to return.
 *
 * @return A new instance of a `CCNxInterestReturn`.
 *
 * Example:
 * @code
 * {
 *     CCNxInterest *interest = ...
 *
 *     CCNxInterestReturn *interestToReturn = ccnxInterest_Create(interest, CCNxInterestReturn_ReturnCode_NoRoute);
 *
 *     ...
 *
 *     ccnxInterestReturn_Release(&interestToReturn);
 * }
 * @endcode
 *
 * @see {@link ccnxInterestReturn_Release}
 */
CCNxInterestReturn *
ccnxInterestReturn_Create(const CCNxInterest *interest, CCNxInterestReturn_ReturnCode returnCode);


/**
 * Retrieve the specified `CCNxInterestReturn`'s {@link CCNxInterestReturn_ReturnCode}
 *
 * @return The return code for the specified CCNxInterestReturn.
 *
 * Example:
 * @code
 * {
 *     CCNxInterest *interest = ccnxInterest_CreateSimple(...);
 *     CCNxInterestReturn *interestReturn = ccnxInterestReturn_Create(interest, CCNxInterestReturn_ReturnCode_NoRoute);
 *
 *     ...
 *
 *     CCNxInterestReturn_ReturnCode returnCode = ccnxInterest_GetReturnCode(interestReturn);
 *
 *     ...
 *
 *     ccnxInterestReturn_Release(&interestReturn);
 * }
 * @endcode
 *
 */
CCNxInterestReturn_ReturnCode
ccnxInterestReturn_GetReturnCode(const CCNxInterestReturn *interestReturn);


// Canonocal
/**
 * Increase the number of references to a `CCNxInterestReturn`.
 *
 * Note that a new `CCNxInterestReturn` is not created,
 * only that the given `CCNxInterestReturn` reference count is incremented.
 * Discard the reference by invoking {@link ccnxInterestReturn_Release}.
 *
 * @param [in] instance A pointer to the original instance.
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     CCNxInterestReturn *reference = ccnxInterestReturn_Acquire(interestReturn);
 *
 *     ...
 *
 *     ccnxInterestReturn_Release(&reference);
 *
 * }
 * @endcode
 *
 * @see {@link ccnxInterestReturn_Release}
 */

CCNxInterestReturn *
ccnxInterestReturn_Acquire(const CCNxInterestReturn *instance);

/**
 * Release a previously acquired reference to the specified instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] instanceP A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     CCNxInterestReturn *reference = ccnxInterestReturn_Acquire(contentObject);
 *
 *     ...
 *
 *     ccnxInterestReturn_Release(&reference);
 * }
 * @endcode
 *
 * @see {@link ccnxInterestReturn_Acquire}
 */
void
ccnxInterestReturn_Release(CCNxInterestReturn **instanceP);

/**
 * Determine if two `CCNxInterestReturn` instances are equal.
 *
 * The following equivalence relations on non-null `CCNxInterestReturn` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `ccnxInterestReturn_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `ccnxInterestReturn_Equals(x, y)` must return true if and only if
 *        `ccnxInterestReturn_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `ccnxInterestReturn_Equals(x, y)` returns true and
 *        `ccnxInterestReturn_Equals(y, z)` returns true,
 *        then  `ccnxInterestReturn_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `ccnxInterestReturn_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `ccnxInterestReturn_Equals(x, NULL)` must
 *      return false.
 *
 * @param interestReturnA A pointer to a `CCNxInterestReturn` instance.
 * @param interestReturnB A pointer to a `CCNxInterestReturn` instance.
 * @return true if the two `CCNxInterestReturn` instances are equal.
 *
 * Example:
 * @code
 * {
 *
 *     CCNxInterestReturn *interestReturnA = ccnxInterestReturn_Create(interestA, CCNxInterestReturn_ReturnCode_NoRoute);
 *     CCNxInterestReturn *interestReturnB = ccnxInterestReturn_Create(interestA, CCNxInterestReturn_ReturnCode_NoRoute); // same as A
 *     CCNxInterestReturn *interestReturnC = ccnxInterestReturn_Create(interestA, CCNxInterestReturn_ReturnCode_HopLimitExceeded ); // different
 *
 *     if (ccnxInterestReturn_Equals(interestReturnA, interestReturnB)) {
 *         // this is expected...
 *     }
 *
 *     if (ccnxInterestReturn_Equals(interestReturnA, interestReturnC)) {
 *         // this is NOT expected
 *     }
 *
 *     ...
 *
 *     ccnxInterestReturn_Release(&interestReturnA);
 *     ccnxInterestReturn_Release(&interestReturnB);
 *     ccnxInterestReturn_Release(&interestReturnC);
 * }
 * @endcode
 */
bool
ccnxInterestReturn_Equals(const CCNxInterestReturn *a, const CCNxInterestReturn *b);

/**
 * Produce a null-terminated string representation of the specified instance.
 *
 * The result must be freed by the caller via {@link parcMemory_Deallocate()}.
 *
 * @param [in] interestReturn A pointer to the instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, nul-terminated C string that must be deallocated via `parcMemory_Deallocate()`.
 *
 * Example:
 * @code
 * {
 *     CCNxInterestReturn *interestReturn = ccnxInterestReturn_Create(...);
 *
 *     char *string = ccnxInterestReturn_ToString(interestReturn);
 *
 *     if (string != NULL) {
 *         printf("InterestReturn looks like: %s\n", string);
 *         parcMemory_Deallocate(string);
 *     } else {
 *         printf("Cannot allocate memory\n");
 *     }
 *
 *     ccnxInterestReturn_Release(&instance);
 * }
 * @endcode
 *
 */
char *
ccnxInterestReturn_ToString(const CCNxInterestReturn *interestReturn);

/**
 * Assert that an instance of `CCNxInterestReturn` is valid.
 *
 * If the instance is not valid, terminate via {@link trapIllegalValue}
 *
 * Valid means the internal state of the type is consistent with its
 * required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] interestReturn A pointer to the instance to check.
 *
 * Example:
 * @code
 * {
 *     CCNxInterestReturn *interestReturn = ccnxInterest_Create(...);
 *     ccnxInterestReturn_AssertValid(interestReturn);
 *
 *     ...
 *
 *     ccnxInterestReturn_Release(&instance);
 * }
 * @endcode
 */
void
ccnxInterestReturn_AssertValid(const CCNxInterestReturn *interestReturn);
#endif //libccnx_ccnx_InterestReturn_h
