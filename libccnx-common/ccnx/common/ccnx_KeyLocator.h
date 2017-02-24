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
 * @file ccnx_KeyLocator.h
 * @ingroup Signature
 * @brief A `CCNxKeyLocator` encapsulates the information and/or data necessary to retrieve a `PARCKey`.
 *
 * There are at least two ways in which a `PARCKey` can be instantiated:
 *
 *   (1) By embedding and subsequently extracting the raw key data in a message, and
 *   (2) By specifying a link for a key so that an interest can be issued to obtain the key content
 *
 * A key locator encapsulates both methods. The API provides functions to create key locators
 * for each key retrieval type and use them to retrieve `PARCKey` methods.
 *
 */
#ifndef libccnx_ccnx_KeyLocator_h
#define libccnx_ccnx_KeyLocator_h

#include <stdbool.h>

#include <ccnx/common/ccnx_Link.h>

#include <parc/security/parc_Key.h>

/**
 * @typedef CCNxKeyLocatorType
 * @brief Locator types for finding keys.
 */
typedef enum {
    CCNxKeyLocatorType_None = 0,
    CCNxKeyLocatorType_Link = 1,
    CCNxKeyLocatorType_Key = 2
} CCNxKeyLocatorType;

struct ccnx_key_locator;

/**
 * @typedef CCNxKeyLocator
 * @brief A `CCNxKeyLocator` encapsulates the information and/or data necessary to retrieve a {@link PARCKey}.
 */
typedef struct ccnx_key_locator CCNxKeyLocator;

/**
 * Create a `CCNxKeyLocator` instance from a {@link PARCKey} instance.
 *
 * @param [in] key The `PARCKey` instance which is used to construct the `CCNxKeyLocator` instance.
 *
 * @return `CCNxKeyLocator` A new `CCNxKeyLocator` instance.
 *
 * Example:
 * @code
 * {
 *     PARCKey *key = ...
 *     CCNxKeyLocator *keyLocator = ccnxKeyLocator_CreateFromKey(key);
 *     // use the keyLocator instance
 * }
 * @endcode
 */
CCNxKeyLocator *ccnxKeyLocator_CreateFromKey(PARCKey *key);

/**
 * Create a `CCNxKeyLocator` instance from a {@link CCNxName} instance.
 *
 * @param [in] keyLink The `CCNxLink` instance used to create the `CCNxKeyLocator` instance
 *
 * @return `CCNxKeyLocator` A new `CCNxKeyLocator` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_CreateFromCString("lci//name");
 *     CCNxLink *keyURILink = ccnxLink_Create(name, NULL, NULL);
 *
 *     CCNxKeyLocator *keyLocator = ccnxKeyLocator_CreateFromKeyLink(keyURILink);
 *     // use the keyLocator instance
 * }
 * @endcode
 */
CCNxKeyLocator *ccnxKeyLocator_CreateFromKeyLink(CCNxLink *keyLink);

/**
 * Retrieve the {@link CCNxKeyLocatorType} associated with this `CCNxKeyLocator`.
 *
 * The locator type will specify one of the three methods used to obtain a
 * PARCKey: (1) embedded keys, (2) certificates, or (3) key (content) names.
 *
 * @param [in] keyLocator The `CCNxKeyLocator` instance from which the location
 *                        type is retrieved.
 *
 * @return A `CCNxKeyLocatorType` value.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxLink_Create("lci//name");
 *     CCNxLink *keyURILink = ccnxName_CreateFromCString(name, NULL, NULL);
 *
 *     CCNxKeyLocator *keyLocator = ccnxKeyLocator_CreateFromKeyLink(keyURILink);
 *     CCNxKeyLocatorType locatorType = ccnxKeyLocator_GetType(keyLocator);
 *     // use the locator type
 * }
 * @endcode
 */
CCNxKeyLocatorType ccnxKeyLocator_GetType(const CCNxKeyLocator *keyLocator);

/**
 * Determine if the key locator type is type `CCNxKeyLocatorType_Key`.
 *
 * @param [in] keyLocator `CCNxKeyLocator` instance being examined.
 *
 * @return true If the key locator type is type `CCNxKeyLocatorType_Key`
 * @return false Otherwise
 *
 * Example:
 * @code
 * {
 *     ...
 *     PARCKey *key = ...
 *     CCNxKeyLocator *keyLocator = ccnxKeyLocator_CreateFromKey(key);
 *     bool isKeyType = ccnxKeyLocator_IsKey(keyLocator);
 *     // isKeyType will be true
 * }
 * @endcode
 */
bool ccnxKeyLocator_IsKey(const CCNxKeyLocator *keyLocator);

/**
 * Determine if the key locator type is type `CCNxKeyLocatorType_Link`.
 *
 * @param [in] keyLocator `CCNxKeyLocator` instance being examined.
 *
 * @return true If the key locator type is type `CCNxKeyLocatorType_Link`
 * @return false Otherwise
 *
 * Example:
 * @code
 * {
 *     ...
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/name");
 *     CCNxLink *keyURILink = ccnxLink_Create(name, NULL, NULL);
 *     CCNxKeyLocator *keyLocator = ccnxKeyLocator_CreateFromKeyLink(keyURILink);
 *     bool isKeyNameType = ccnxKeyLocator_IsKeyLink(keyLocator);
 *     // isKeyNameType will be true
 * }
 * @endcode
 */
bool ccnxKeyLocator_IsKeyLink(const CCNxKeyLocator *keyLocator);

/**
 * Produce a null-terminated C-string representation of the specified instance.
 *
 * The non-null result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] instance A pointer to the instance.
 *
 * @return NULL  Memory could not be allocated.
 * @return non-NULL A null-terminated string that must be deallocated via `parcMemory_Deallocate`.
 *
 * Example:
 * @code
 * {
 *     ...
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/name");
 *     CCNxKeyLocator *keyLocator = ccnxKeyLocator_FromKeyName(name);
 *     char *stringRep = ccnxKeyLocator_ToString(keyLocator);
 * }
 * @endcode
 */
char *ccnxKeyLocator_ToString(const CCNxKeyLocator *instance);

/**
 * Retrieve the {@link CCNxLink} instance from the specified `CCNxKeyLocator` instance.
 *
 * The type of the key locator must be `CCNxKeyLocatorType_Link` prior to invocation,
 * else a trap will be invoked.
 *
 * @param [in] keyLocator The `CCNxKeyLocator` from which the name is extracted.
 *
 * @return A non-NULL `CCNxName` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name = ccnxName_Create("lci//name");
 *     CCNxLink *keyURILink = ccnxLink_CreateFromURI(name, NULL, NULL);
 *
 *     CCNxKeyLocator *keyLocator = ccnxKeyLocator_CreateFromKeyLink(keyURILink);
 *     CCNxLink *copy = ccnxKeyLocator_GetKeyLink(keyLocator);
 *     // use the link copy
 * }
 * @endcode
 */
CCNxLink *ccnxKeyLocator_GetKeyLink(const CCNxKeyLocator *keyLocator);

/**
 * Retrieve the {@link PARCKey} instance from the specified `CCNxKeyLocator` instance.
 *
 * The type of the key locator must be `CCNxKeyLocatorType_Key` prior to invocation,
 * else a trap will be invoked.
 *
 * @param [in] keyLocator The `CCNxKeyLocator` from which the key is extracted.
 *
 * @return A non-NULL `PARCKey` instance.
 *
 * Example:
 * @code
 * {
 *     PARCKey *key = ...;
 *     CCNxKeyLocator *keyLocator = ccnxKeyLocator_FromKey(key);
 *     CCNxKey *copy = ccnxKeyLocator_GetKey(keyLocator);
 *     // use the key copy
 * }
 * @endcode
 */
PARCKey *ccnxKeyLocator_GetKey(const CCNxKeyLocator *keyLocator);

/**
 * Determine if two `CCNxKeyLocators` are equal.
 *
 * The following equivalence relations on non-null `CCNxKeyLocator` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, `ccnxKeyLocator_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `ccnxKeyLocator_Equals(x, y)` must return true if and only if
 *        `ccnxKeyLocator_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `ccnxKeyLocator_Equals(x, y)` returns true and
 *        `ccnxKeyLocator_Equals(y, z)` returns true,
 *        then  `ccnxKeyLocator_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `ccnxKeyLocator_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `ccnxKeyLocator_Equals(x, NULL)` must return false.
 *
 * @param a A pointer to a `CCNxKeyLocator` instance.
 * @param b A pointer to a `CCNxKeyLocator` instance.
 * @return True if the referenced `CCNxKeyLocators` are equal.
 *
 * Example:
 * @code
 * {
 *     CCNxName *name1 = ccnxLink_Create("lci//name");
 *     CCNxLink *keyURILink1 = ccnxName_CreateFromCString(name1, NULL, NULL);
 *
 *     CCNxKeyLocator *keyLocator1 = ccnxKeyLocator_FromKeyName(keyURILink1);
 *
 *     CCNxName *name2 = ccnxLink_Create("lci//name");
 *     CCNxLink *keyURILink2 = ccnxName_CreateFromCString(name2, NULL, NULL);
 *
 *     CCNxKeyLocator *keyLocator2 = ccnxKeyLocator_FromKeyName(keyURILink2);
 *
 *     if (ccnxKeyLocator_Equals(keyLocator1, keyLocator2)) {
 *          // true
 *      } else {
 *          // false
 *      }
 * }
 * @endcode
 */
bool ccnxKeyLocator_Equals(const CCNxKeyLocator *a, const CCNxKeyLocator *b);

/**
 * Create a copy of the given `CCNxKeyLocator` instance.
 *
 * This creates a deep copy of the `CCNxKeyLocator` instance, acquiring handles to internal object
 * references when needed.
 *
 * @param [in] original The `CCNxKeyLocator` instance which is being copied
 *
 * @return `CCNxKeyLocator` A copy of the specified `CCNxKeyLocator`
 *
 * Example:
 * @code
 * {
 *     ...
 *     CCNxName *keyURIName = ccnxName_CreateFromCString("lci://name");
 *     CCNxKeyLocator *keyLocator = ccnxKeyLocator_FromKeyName(keyURIName);
 *     CCNxKeyLocator *copy = ccnxKeyLocator_Copy(keyLocator);
 *     // use the copied instance
 * }
 * @endcode
 */
CCNxKeyLocator *ccnxKeyLocator_Copy(const CCNxKeyLocator *original);

/**
 * Increase the number of references to a `CCNxKeyLocator`.
 *
 * Note that new `CCNxKeyLocator` is not created,
 * only that the given `CCNxKeyLocator` reference count is incremented.
 * Discard the reference by invoking {@link ccnxKeyLocator_Release}.
 *
 * @param instance A pointer to the original instance.
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxKeyLocator *keyLocator = ccnxKeyLocator_Acquire(instance);
 *
 *     ccnxKeyLocator_Release(&keyLocator);
 *
 * }
 * @endcode
 *
 * @see `ccnxKeyLocator_Release`
 */
CCNxKeyLocator *ccnxKeyLocator_Acquire(const CCNxKeyLocator *instance);

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
 * @param [in,out] object A pointer to a pointer to the instance to release.
 *
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxKeyLocator *cert = ccnxKeyLocator_Acquire(instance);
 *
 *     ccnxKeyLocator_Release(&cert);
 *
 * }
 * @endcode
 */
void ccnxKeyLocator_Release(CCNxKeyLocator **object);

#ifdef Libccnx_DISABLE_VALIDATION
#  define ccnxKeyLocator_OptionalAssertValid(_instance_)
#else
#  define ccnxKeyLocator_OptionalAssertValid(_instance_) ccnxKeyLocator_AssertValid(_instance_)
#endif
/**
 * Check that the pointer to the `KeyLocator` is valid. It should be non-null,
 * and any required referenced data should be valid.
 *
 * @param [in] object A pointer to the instance to check.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     ccnxKeyLocator_AssertValid(keyLocator);
 *
 * }
 * @endcode
 */
void ccnxKeyLocator_AssertValid(const CCNxKeyLocator *object);
#endif // libccnx_ccnx_KeyLocator_h
