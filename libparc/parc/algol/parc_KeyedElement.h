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
 * @file parc_KeyedElement.h
 * @ingroup datastructures
 * @brief A Pointer and a Key
 *  The `PARCKeyedElement` is a simple pointer and key tuple.
 *
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
#ifndef libparc_parc_KeyedElement_h
#define libparc_parc_KeyedElement_h

#include <stdint.h>
#include <stdlib.h>

struct parc_keyed_element;

/**
 * A `PARCKeyedElement` is a tuple consisting of an address and a key+len.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
typedef struct parc_keyed_element PARCKeyedElement;

/**
 * Create a `PARCKeyedElement`.  Note that the key will be copied (size keylen) while the data will just be
 * referenced.  The key copy will be released when the KeyedElement is destroyed.
 *
 * @param [in] data The data we want to associate with a key.
 * @param [in] key A pointer to the key data (will be copied).
 * @param [in] keylen The length of the keydata.
 * @return The new instance of `PARCKeyedElement`.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCKeyedElement *parcKeyedElement_Create(void *data, const void *key, size_t keylen);

/**
 * Destroy a `PARCKeyedElement`.
 *
 * If the Free functions were passed to the constructor they will be called if
 * not NULL. *
 * @param [in,out] keyedElementPointer A pointer to the pointer to the `PARCKeyedElement` to be destroyed.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcKeyedElement_Destroy(PARCKeyedElement **keyedElementPointer);
/**
 * Set the data of a `PARCKeyedElement`.
 *
 * @param [in,out] keyedElement The pointer to the `PARCKeyedElement` whose data will be reset to @p data.
 * @param [in] data The data to put be into @p keyedElement
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcKeyedElement_SetData(PARCKeyedElement *keyedElement, void *data);

/**
 * Get the data of a `PARCKeyedElement`.
 *
 * @param [in] keyedElement The pointer to the `PARCKeyedElement` whose data will be retrieved.
 * @return A pointer to the retrieved data.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcKeyedElement_GetData(PARCKeyedElement *keyedElement);

/**
 * Retrieve the key of a `PARCKeyedElement`.
 *
 * @param [in] keyedElement A pointer to the `PARCKeyedElement` whose key should be retreieved.
 *
 * @return A pointer to the `PARCKeyedElement`'s key.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *parcKeyedElement_GetKey(PARCKeyedElement *keyedElement);

/**
 * Return the size of the `PARCKeyedElement`'s key.
 *
 * @param [in] keyedElement A pointer to the `PARCKeyedElement` whose key length should be returned.
 *
 * @return The length of the retrieved key.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
size_t parcKeyedElement_GetKeyLen(PARCKeyedElement *keyedElement);
#endif // libparc_parc_KeyedElement_h
