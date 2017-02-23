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
 * @file ccnx_TlvDictionary.h
 * @brief Stores pointers to PARCBuffers indexed by keys
 *
 * A message dictionary stores each field of a message in an array entry.  The user of the dictionary needs to
 * have a schema so it knows which array entry is which field.
 *
 * A message dictionary carries two distinguished fields that are not part of the array.  The MessageType is
 * Interest, ContentObject, or Control.  The SchemaVersion is 0 or 1.  These fields are independent of
 * anything that is in the dictionary.
 *
 * The message dictionary creates two arrays.  The first array is an array of PARCBuffer.  The second array
 * is a list of (type, PARCBuffer).  This structure is designed such that well-known TLV keys
 * are stored in the first array under well-known indicies.
 *
 * unknown TLV keys are stored in the second array, where each list corresponds to a TLV container.
 * For example, an unknown TLV type found under the Content Object Metadata container would
 * go in a list corresponding to the Metadata container and they be added to the list with the
 * pair (type, PARCBuffer), where 'type' is the unknown TLV type.
 *
 */

#ifndef libccnx_ccnx_TlvDictionary_h
#define libccnx_ccnx_TlvDictionary_h

#include <stdbool.h>
#include <sys/time.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_JSON.h>

#include <ccnx/common/internal/ccnx_MessageInterface.h>

#include <ccnx/common/ccnx_Name.h>
#include <ccnx/common/codec/ccnxCodec_NetworkBuffer.h>


struct ccnx_tlv_dictionary;
typedef struct ccnx_tlv_dictionary CCNxTlvDictionary;

typedef enum {
    CCNxTlvDictionary_SchemaVersion_V0 = 0,
    CCNxTlvDictionary_SchemaVersion_V1 = 1,
} CCNxTlvDictionary_SchemaVersion;


/**
 * Creates a new TLV dictionary with the given size
 *
 * There will be 'bufferCount' array elements of type Buffer and
 * 'listCount' elements of type List.  Each array is indexed from 0.
 *
 * @param [in] bufferCount The number of Buffer elements to allocate within the dictionary.
 * @param [in] listCount The number of List elements to allocate within the dictionary.
 *
 * @return NULL A new CCNxTlvDictionary object could not be allocated.
 * @return CCNxTlvDictionary A new CCNxTlvDictionary instance with bufferCount Buffer and listCount List elements.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_PutBuffer(dict, 3, nameBuffer);
 *     rtatlvDictionary_PutListBuffer(dict, 2, unknownType, unknownBuffer);
 * }
 * @endcode
 */
CCNxTlvDictionary *ccnxTlvDictionary_Create(size_t bufferCount, size_t listCount);

/**
 * Acquire a handle to the CCNxTlvDictionary instance.
 *
 * Note that new instance is not created,
 * only that the given instance's reference count is incremented.
 * Discard the reference by invoking `ccnxTlvDictionary_Release()`.
 *
 * @param [in] dictionary A pointer to the original instance.
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Acquire(instance);
 *
 *     ccnxTlvDictionary_Release(&dict);
 * }
 * @endcode
 */
CCNxTlvDictionary *ccnxTlvDictionary_Acquire(const CCNxTlvDictionary *dictionary);

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
 * @param [in] dictionaryPtr A pointer to a pointer to the instance to release.
 *
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Acquire(instance);
 *
 *     ccnxTlvDictionary_Release(&dict);
 * }
 * @endcode
 */
void ccnxTlvDictionary_Release(CCNxTlvDictionary **dictionaryPtr);

/**
 * Adds a buffer to a dictionary entry
 *
 * Stores a reference count to the buffer
 *
 * @param [in] dictionary An CCNxTlvDictionary instance to which the Buffer entry will be added.
 * @param [in] key The integer key that is to be associated with the new Buffer entry.
 * @param [in] buffer The Buffer entry value to be inserted into the dictionary.
 *
 * @return true Key was not previously set
 * @return false Key already has a buffer assigned to it
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     PARCBuffer *buffer = parcBuffer_Allocate(1);
 *     ccnxTlvDictionary_PutBuffer(dict, 1, buffer);
 *     // use the dictionary as needed
 * }
 * @endcode
 */
bool ccnxTlvDictionary_PutBuffer(CCNxTlvDictionary *dictionary, uint32_t key, const PARCBuffer *buffer);

/**
 * Determine if the value associated with the specified key is a Buffer.
 *
 * The key must be within the interval [0, bufferCount] for the dictionary.
 *
 * @param [in] dictionary The dictionary instance which will be examined.
 * @param [in] key The key to use when indexing the dictionary.
 *
 * @return true The value associated with the key is of type Buffer.
 * @return false The value associated with the key is -not- of type Buffer.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     PARCBuffer *buffer = parcBuffer_Allocate(1);
 *     ccnxTlvDictionary_PutBuffer(dict, 1, buffer);
 *     bool truthy = ccnxTlvDictionary_IsValueBuffer(dict, 1);
 *     // truthy will be true
 * }
 * @endcode
 */
bool ccnxTlvDictionary_IsValueBuffer(const CCNxTlvDictionary *dictionary, uint32_t key);

// caw TODO
bool ccnxTlvDictionary_IsValueObject(const CCNxTlvDictionary *dictionary, uint32_t key);

/**
 * Retrieves an entry from the dictionary from the specified key.
 *
 * @param [in] dictionary The dictionary instance which will be examined.
 * @param [in] key The key to use when indexing the dictionary.
 *
 * @return NULL if key not found
 * @return non-null The desired key
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     PARCBuffer *buffer = parcBuffer_Allocate(1);
 *     ccnxTlvDictionary_PutBuffer(dict, 1, buffer);
 *     PARCBuffer *copy = ccnxTlvDictionary_GetBuffer(dict, 1);
 *     // copy will be equal to the buffer instance
 * }
 * @endcode
 */
PARCBuffer *ccnxTlvDictionary_GetBuffer(const CCNxTlvDictionary *dictionary, uint32_t key);

/**
 * Put a new integer value in the dictionary, overwriting the old value if the key is
 * already present.
 *
 * You can put an integer value many times, its OK to overwrite.
 * They key must be UNSET or INTEGER, you cannot overwrite a different type
 * The integer will be encoded as per the schema, it will not necessarily be an 8-byte field.
 *
 * @param [in] dictionary The dictionary instance to be modified
 * @param [in] key The key used when indexing the dictionary
 * @param [in] value The new value to insert into the dictionary assoicated with the above key
 *
 * @return true If the put/update was successful.
 * @return false Otherwise (e.g., not UNSET/INTEGER type)
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     bool success = ccnxTlvDictionary_PutInteger(dict, 2, 1337);
 *     // success will be true since key 2 was UNSET
 * }
 * @endcode
 */
bool ccnxTlvDictionary_PutInteger(CCNxTlvDictionary *dictionary, uint32_t key, const uint64_t value);

/**
 * Determine if the value associated with the specified key is an Integer.
 *
 * The key must be within the interval [0, bufferCount] for the dictionary.
 *
 * @param [in] dictionary The dictionary instance which will be examined.
 * @param [in] key The key to use when indexing the dictionary.
 *
 * @return true The value associated with the key is of type INTEGER.
 * @return false The value associated with the key is -not- of type INTEGER.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_PutInteger(dict, 2, 1337);
 *     bool truthy = ccnxTlvDictionary_IsValueInteger(dict, 2);
 *     // truthy will be true
 * }
 * @endcode
 */
bool ccnxTlvDictionary_IsValueInteger(const CCNxTlvDictionary *dictionary, uint32_t key);

/**
 * Returns an integer value stored in a key
 *
 * Will trapIllegalValue if the given key is not of type Integer.  You should use
 * ccnxTlvDictionary_IsValueInteger before calling.  An unset key is not of type Integer.
 *
 * @param [in] dictionary The dictionary to check
 * @param [in] key The key to retrieve
 *
 * @return number The value stored under the key
 *
 * Example:
 * @code
 *      static uint32_t
 *      _fetchUint32(const CCNxTlvDictionary *interestDictionary, uint32_t key, uint32_t defaultValue)
 *      {
 *          if (ccnxTlvDictionary_IsValueInteger(interestDictionary, key)) {
 *              return (uint32_t) ccnxTlvDictionary_GetInteger(interestDictionary, key);
 *          }
 *          return defaultValue;
 *      }
 * @endcode
 */
uint64_t ccnxTlvDictionary_GetInteger(const CCNxTlvDictionary *dictionary, uint32_t key);

/**
 * Put a new CCNxName into the `CCNxTlvDictionary` instance.
 *
 * You can only set a Name field once. The key must be UNSET.
 *
 * @param [in] dictionary The dictionary instance to be modified
 * @param [in] key The key used when indexing the dictionary
 * @param [in] name The new CCNxName value to insert into the dictionary assoicated with the above key
 *
 * @return true If the put/update was successful.
 * @return false Otherwise (e.g., not UNSET type)
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar");
 *     bool success = ccnxTlvDictionary_PutName(dict, 2, name);
 *     // success will be true since key 2 was UNSET
 * }
 * @endcode
 */
bool ccnxTlvDictionary_PutName(CCNxTlvDictionary *dictionary, uint32_t key, const CCNxName *name);

/**
 * Determine if the value associated with the specified key is a CCNxName.
 *
 * The key must be within the interval [0, bufferCount] for the dictionary.
 *
 * @param [in] dictionary The dictionary instance which will be examined.
 * @param [in] key The key to use when indexing the dictionary.
 *
 * @return true The value associated with the key is of type NAME.
 * @return false The value associated with the key is -not- of type NAME.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar");
 *     ccnxTlvDictionary_PutName(dict, 2, name);
 *     bool truthy = ccnxTlvDictionary_IsValueName(dict, 2);
 *     // truthy will be true since a name was inserted with key=2
 * }
 * @endcode
 */
bool ccnxTlvDictionary_IsValueName(const CCNxTlvDictionary *dictionary, uint32_t key);

/**
 * Retrieve the CCNxName instance associated with the specified key.
 *
 * The key must be within the interval [0, bufferCount] for the dictionary.
 * The entry is expected to be of type NAME, and will return NULL if not.
 *
 * @param [in] dictionary The dictionary instance which will be queried.
 * @param [in] key The key to use when indexing the dictionary.
 *
 * @return NULL The entry associated with the key is not of type NAME.
 * @return CCNxName A CCNxName instance associated with the specified key.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     CCNxName *name = ccnxName_CreateFromCString("lci:/foo/bar");
 *     ccnxTlvDictionary_PutName(dict, 2, name);
 *     CCNxName *copy = ccnxTlvDictionary_GetName(dict, 2);
 *     // do something with copy
 * }
 * @endcode
 */
CCNxName *ccnxTlvDictionary_GetName(const CCNxTlvDictionary *dictionary, uint32_t key);

/**
 * Insert a `CCNxCodecNetworkBufferIoVec` instance into the dictionary.
 *
 * Stores a scatter/gather network buffer. Could be either the wire format we recieve
 * or the wire format we're about to send. The resulting entry type will be IOVEC.
 *
 * @param [in] dictionary The dictionary instance to be modified
 * @param [in] key The key used when indexing the dictionary
 * @param [in] vec The new CCNxCodecNetworkBufferIoVec value to insert into the dictionary assoicated with the above key
 *
 * @return true If the put/update was successful.
 * @return false Otherwise (e.g., not UNSET type)
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     CCNxCodecNetworkBuffer *buffer = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
 *     CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecNetworkBuffer_CreateIoVec(buffer);
 *     bool success = ccnxTlvDictionary_PutName(dict, 2, iovec);
 *     // success will be true since key 2 was UNSET
 * }
 * @endcode
 */
bool ccnxTlvDictionary_PutIoVec(CCNxTlvDictionary *dictionary, uint32_t key, const CCNxCodecNetworkBufferIoVec *vec);

/**
 * Determine if the value associated with the specified key is a CCNxCodecNetworkBufferIoVec.
 *
 * @param [in] dictionary The dictionary instance to be examined
 * @param [in] key The key used when indexing the dictionary
 *
 * @return true The TLV dictionary has the given key and it is of type IOVEC
 * @return false Otherwise
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     CCNxCodecNetworkBuffer *buffer = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
 *     CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecNetworkBuffer_CreateIoVec(buffer);
 *     ccnxTlvDictionary_PutName(dict, 2, iovec);
 *     bool truthy = ccnxTlvDictionary_IsValueIoVec(dict, 2);
 *     // truthy will be true since key 2 was IOVEC and previously inserted
 * }
 * @endcode
 */
bool ccnxTlvDictionary_IsValueIoVec(const CCNxTlvDictionary *dictionary, uint32_t key);

/**
 * Retrieve the CCNxCodecNetworkBufferIoVec instance associated with the specified key.
 *
 * The key must be within the interval [0, bufferCount] for the dictionary.
 * The entry is expected to be of type IOVEC, and will return NULL if not.
 *
 * @param [in] dictionary The dictionary instance which will be queried.
 * @param [in] key The key to use when indexing the dictionary.
 *
 * @return NULL The entry associated with the key is not of type IOVEC.
 * @return CCNxCodecNetworkBufferIoVec A CCNxCodecNetworkBufferIoVec instance associated with the specified key.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     CCNxCodecNetworkBuffer *buffer = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
 *     CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecNetworkBuffer_CreateIoVec(buffer);
 *     ccnxTlvDictionary_PutName(dict, 2, iovec);
 *     CCNxCodecNetworkBufferIoVec *copy = ccnxTlvDictionary_GetIoVec(dict, 2);
 *     // do something with copy
 * }
 * @endcode
 */
CCNxCodecNetworkBufferIoVec *ccnxTlvDictionary_GetIoVec(const CCNxTlvDictionary *dictionary, uint32_t key);

/**
 * Insert a new List item into the dictionary.
 *
 * The key must be within the interval [0, bufferCount] for the dictionary.
 * You can only set a Buffer field once. The key must be UNSET.
 *
 * @param [in] dictionary The dictionary instance to be modified
 * @param [in] listKey The list key used when indexing the dictionary lists
 * @param [in] key The key type of the element being inserted into the list
 * @param [in] buffer The new PARCBuffer value to insert into the dictionary list indexed by the listKey
 *
 * @return true If the put/update was successful.
 * @return false Otherwise (e.g., not UNSET type)
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     PARCBuffer *buffer = parcBuffer_Allocate(1);
 *     bool success = ccnxTlvDictionary_PutListBuffer(dict, 1, 1, buffer);
 *     // success will be true since list key 1 was UNSET, and the new item will have key type 1
 * }
 * @endcode
 */
bool ccnxTlvDictionary_PutListBuffer(CCNxTlvDictionary *dictionary, uint32_t listKey, uint32_t key, const PARCBuffer *buffer);

/**
 * Insert a new `PARCJSON` instance into the dictionary.
 *
 * The caller may destroy its reference to JSON since the type is deep-copied internally.
 * The key must be within the dictionary, and the entry must be UNSET.
 *
 * @param [in] dictionary The dictionary instance to be modified
 * @param [in] key The key used when indexing the dictionary
 * @param [in] json The new PARCJSON value to insert into the dictionary associated with the above key
 *
 * @return true If the put/update was successful.
 * @return false Otherwise (e.g., not UNSET type)
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     PARCJSON *json = ccnxJson_CreateNumber(1);
 *     bool success = ccnxTlvDictionary_PutJson(dict, 1, json);
 *     // success will be true since the key was UNSET
 * }
 * @endcode
 */
bool ccnxTlvDictionary_PutJson(CCNxTlvDictionary *dictionary, uint32_t key, const PARCJSON *json);

/**
 * Determine if the value associated with the specified key is a `PARCJSON` instance.
 *
 * @param [in] dictionary The dictionary instance to be examined
 * @param [in] key The key used when indexing the dictionary
 *
 * @return true The TLV dictionary has the given key and it is of type JSON
 * @return false Otherwise
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     PARCJSON *json = ccnxJson_CreateNumber(1);
 *     ccnxTlvDictionary_PutJson(dict, 1, json);
 *     bool truthy = ccnxTlvDictionary_IsValueJson(dict, 1);
 *     // truthy will be true since the JSON object was previously inserted
 * }
 * @endcode
 */
bool ccnxTlvDictionary_IsValueJson(const CCNxTlvDictionary *dictionary, uint32_t key);

/**
 * Retrieve the `PARCJSON` instance associated with the specified key.
 *
 * The key must be within the interval [0, bufferCount] for the dictionary.
 * The entry is expected to be of type JSON, and will return NULL if not.
 *
 * @param [in] dictionary The dictionary instance which will be queried.
 * @param [in] key The key to use when indexing the dictionary.
 *
 * @return NULL The entry associated with the key is not of type JSON.
 * @return PARCJSON A PARCJSON instance associated with the specified key.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     PARCJSON *json = ccnxJson_CreateNumber(1);
 *     ccnxTlvDictionary_PutJson(dict, 1, json);
 *     PARCJSON *copy = ccnxTlvDictionary_GetJson(dict, 1);
 *     // do something with copy
 * }
 * @endcode
 */
PARCJSON *ccnxTlvDictionary_GetJson(const CCNxTlvDictionary *dictionary, uint32_t key);

/**
 * Insert a new `PARCJSON` instance into the dictionary.
 *
 * The caller may destroy its reference to PARCObject since the type is deep-copied internally.
 * The key must be within the dictionary, and the entry must be UNSET.
 *
 * @param [in] dictionary The dictionary instance to be modified
 * @param [in] key The key used when indexing the dictionary
 * @param [in] json The new PARCObject value to insert into the dictionary associated with the above key
 *
 * @return true If the put/update was successful.
 * @return false Otherwise (e.g., not UNSET type)
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     PARCObject *object = ccnxName_CreateFromCString("ccnx/test");
 *     bool success = ccnxTlvDictionary_PutObject(dict, 1, object);
 *     // success will be true since the key was UNSET
 * }
 * @endcode
 */
bool ccnxTlvDictionary_PutObject(CCNxTlvDictionary *dictionary, uint32_t key, const PARCObject *json);

/**
 * Determine if the value associated with the specified key is a `PARCObject` instance.
 *
 * @param [in] dictionary The dictionary instance to be examined
 * @param [in] key The key used when indexing the dictionary
 *
 * @return true The TLV dictionary has the given key and it is of type PARCObject
 * @return false Otherwise
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     PARCObject *object = ccnxName_CreateFromCString("ccnx/test");
 *     ccnxTlvDictionary_PutObject(dict, 1, object);
 *     bool truthy = ccnxTlvDictionary_IsValueObject(dict, 1);
 *     // truthy will be true since the PARCObject was previously inserted
 * }
 * @endcode
 */
bool ccnxTlvDictionary_IsValueObject(const CCNxTlvDictionary *dictionary, uint32_t key);

/**
 * Retrieve the `PARCObject` instance associated with the specified key.
 *
 * The key must be within the interval [0, bufferCount] for the dictionary.
 * The entry is expected to be of type PARCObject, and will return NULL if not.
 *
 * @param [in] dictionary The dictionary instance which will be queried.
 * @param [in] key The key to use when indexing the dictionary.
 *
 * @return NULL The entry associated with the key is not of type PARCObject.
 * @return PARCObject A PARCObject instance associated with the specified key.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     PARCObject *object = ccnxName_CreateFromCString("ccnx/test");
 *     ccnxTlvDictionary_PutObject(dict, 1, object);
 *     PARCObject *copy = ccnxTlvDictionary_GetObject(dict, 1);
 *     // do something with copy
 * }
 * @endcode
 */
PARCObject *ccnxTlvDictionary_GetObject(const CCNxTlvDictionary *dictionary, uint32_t key);

/**
 * Fetches a buffer from the ordinal position 'listItem' from the list key 'key'
 *
 * The entry 'key' must be type list.
 *
 * @param [in] dictionary The dictionary instance being examined
 * @param [in] listKey The key used to identify the list to be searched
 * @param [in] listPosition The index within the target list of the dictionary
 * @param [out] bufferPtr If position is found, the buffer at that position
 * @param [out] keyPtr If position is found, the key of the buffer
 *
 * @return true position found
 * @return false position not found
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     PARCBuffer *buffer = parcBuffer_Allocate(1);
 *     ccnxTlvDictionary_PutListBuffer(dict, 1, 1, buffer);
 *     PARCBuffer *copy = NULL;
 *     uint32_t key = 0;
 *     ccnxTlvDictionary_ListGetByPosition(dict, 1, 0, &copy, &key);
 *     // use the copy and key as necessary
 * }
 * @endcode
 */
bool ccnxTlvDictionary_ListGetByPosition(const CCNxTlvDictionary *dictionary, uint32_t listKey, size_t listPosition, PARCBuffer **bufferPtr, uint32_t *keyPtr);

/**
 * Returns the first buffer in the list identified by 'listkey' with the buffer type 'type'
 *
 * @param [in] dictionary The dictionary instance being examined
 * @param [in] listKey The key used to index into the dictionary lists
 * @param [in] type The type of element used to search within the dictionary list
 *
 * @return PARCBuffer* Pointer to the first PARCBuffer instance whose type matches the type argument in the target list
 * @return NULL If no element in the target list has the specified type.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     PARCBuffer *buffer = parcBuffer_Allocate(1);
 *     ccnxTlvDictionary_PutListBuffer(dict, 1, 1, buffer);
 *     PARCBuffer *copy = ccnxTlvDictionary_ListGetByType(dict, 1, 1);
 *     // copy and buffer will have equal PARCBuffer values
 * }
 * @endcode
 */
PARCBuffer *ccnxTlvDictionary_ListGetByType(const CCNxTlvDictionary *dictionary, uint32_t listKey, uint32_t type);

/**
 * Retrieve the number of elements in the list identified by 'key'
 *
 * The dictionary entry 'key' must be of type list.
 *
 * @param [in] dictionary The dictionary instance to be examined
 * @param [in] listKey The key used to index into the list whose size will be checked
 *
 * @return The size of the list associated with the given list key
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     PARCBuffer *buffer = parcBuffer_Allocate(1);
 *     ccnxTlvDictionary_PutListBuffer(dict, 1, 1, buffer);
 *     size_t listSize = ccnxTlvDictionary_ListSize(dictionary, 1);
 *     // listSize will be 1
 * }
 * @endcode
 */
size_t ccnxTlvDictionary_ListSize(const CCNxTlvDictionary *dictionary, uint32_t listKey);

/**
 * Set the type of message which this dictionary stores/represents to be an Interest.
 *
 * @param [in] dictionary The dictionary instance whose type is to be modified
 * @param [in] schemaVersion The schema version which is used by the dictionary for message encoding.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_SetMessageType_Interest(dict, CCNxTlvDictionary_SchemaVersion_V0);
 *     // use the dictionary
 * }
 * @endcode
 */
void ccnxTlvDictionary_SetMessageType_Interest(CCNxTlvDictionary *dictionary, CCNxTlvDictionary_SchemaVersion schemaVersion);

/**
 * Set the type of message which this dictionary stores/represents to be a ContentObject.
 *
 * @param [in] dictionary The dictionary instance whose type is to be modified
 * @param [in] schemaVersion The schema version which is used by the dictionary for message encoding.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_SetMessageType_ContentObject(dict, CCNxTlvDictionary_SchemaVersion_V1);
 *     // use the dictionary
 * }
 * @endcode
 */
void ccnxTlvDictionary_SetMessageType_ContentObject(CCNxTlvDictionary *dictionary, CCNxTlvDictionary_SchemaVersion schemaVersion);

/**
 * Set the type of message which this dictionary stores/represents to be a Control (message).
 *
 * @param [in] dictionary The dictionary instance whose type is to be modified
 * @param [in] schemaVersion The schema version which is used by the dictionary for message encoding.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_SetMessageType_Control(dict, CCNxTlvDictionary_SchemaVersion_V1);
 *     // use the dictionary
 * }
 * @endcode
 */
void ccnxTlvDictionary_SetMessageType_Control(CCNxTlvDictionary *dictionary, CCNxTlvDictionary_SchemaVersion schemaVersion);

/**
 * Set the type of message which this dictionary stores/represents to be a Manifest (message).
 *
 * @param [in] dictionary The dictionary instance whose type is to be modified.
 * @param [in] schemaVersion The schema version which is used by the dictionary for message encoding.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_SetMessageType_Manifest(dict, CCNxTlvDictionary_SchemaVersion_V1);
 *     // use the dictionary
 * }
 * @endcode
 */
void ccnxTlvDictionary_SetMessageType_Manifest(CCNxTlvDictionary *dictionary, CCNxTlvDictionary_SchemaVersion schemaVersion);

/**
 * Set the type of message which this dictionary stores/represents to be an InterestReturn.
 *
 * @param [in] dictionary The dictionary instance whose type is to be modified
 * @param [in] schemaVersion The schema version which is used by the dictionary for message encoding.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_SetMessageType_InterestReturn(dict, CCNxTlvDictionary_SchemaVersion_V0);
 *     // use the dictionary
 * }
 * @endcode
 */
void ccnxTlvDictionary_SetMessageType_InterestReturn(CCNxTlvDictionary *dictionary, CCNxTlvDictionary_SchemaVersion schemaVersion);


/**
 * Retrieve the schema version `CCNxTlvDictionary_SchemaVersion` used to encode the contents of the dictionary.
 *
 * Currently, only two schema versions are supported: CCNxTlvDictionary_SchemaVersion_V0 and CCNxTlvDictionary_SchemaVersion_V1
 *
 * @param [in] dictionary The dictionary instance being examined
 *
 * @return The `CCNxTlvDictionary_SchemaVersion` version used for encoding.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_SetMessageType_Interest(dict, CCNxTlvDictionary_SchemaVersion_V0);
 *     CCNxTlvDictionary_SchemaVersion ver = ccnxTlvDictionary_GetSchemaVersion(dict);
 *     // ver will be equal to CCNxTlvDictionary_SchemaVersion_V0
 * }
 * @endcode
 */
CCNxTlvDictionary_SchemaVersion ccnxTlvDictionary_GetSchemaVersion(const CCNxTlvDictionary *dictionary);

/**
 * Determine if the specified dictionary represents an Interest message.
 *
 * @param [in] dictionary The dictionary instance being examined.
 *
 * @return true The dictionary represents an Interest message.
 * @return false Otherwise
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_SetMessageType_Interest(dict, CCNxTlvDictionary_SchemaVersion_V0);
 *     bool isInterest = ccnxTlvDictionary_IsInterest(dict);
 *     // isInterest will be true
 * }
 * @endcode
 */
bool ccnxTlvDictionary_IsInterest(const CCNxTlvDictionary *dictionary);

/**
 * Determine if the specified dictionary represents an InterestReturn message.
 *
 * @param [in] dictionary The dictionary instance being examined.
 *
 * @return true The dictionary represents an InterestReturn message.
 * @return false Otherwise
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_SetMessageType_InterestReturn(dict, CCNxTlvDictionary_SchemaVersion_V1);
 *     bool isInterestReturn = ccnxTlvDictionary_IsInterestReturn(dict);
 *     // isInterest will be true
 * }
 * @endcode
 */
bool ccnxTlvDictionary_IsInterestReturn(const CCNxTlvDictionary *dictionary);

/**
 * Determine if the specified dictionary represents a ContentObject message.
 *
 * @param [in] dictionary The dictionary instance being examined.
 *
 * @return true The dictionary represents a ContentObject message.
 * @return false Otherwise
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_SetMessageType_ContentObject(dict, CCNxTlvDictionary_SchemaVersion_V0);
 *     bool isContentObject = ccnxTlvDictionary_IsContentObject(dict);
 *     // isContentObject will be true
 * }
 * @endcode
 */
bool ccnxTlvDictionary_IsContentObject(const CCNxTlvDictionary *dictionary);

/**
 * Determine if the specified dictionary represents a Control message.
 *
 * @param [in] dictionary The dictionary instance being examined.
 *
 * @return true The dictionary represents a Control message.
 * @return false Otherwise
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_SetMessageType_Control(dict, CCNxTlvDictionary_SchemaVersion_V0);
 *     bool isControl = ccnxTlvDictionary_IsControl(dict);
 *     // isControl will be true
 * }
 * @endcode
 */
bool ccnxTlvDictionary_IsControl(const CCNxTlvDictionary *dictionary);

/**
 * Determine if the specified dictionary represents a Manifest message.
 *
 * @param [in] dictionary The dictionary instance being examined.
 *
 * @return true The dictionary represents a Manifest message.
 * @return false Otherwise
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_SetMessageType_Manifest(dict, CCNxTlvDictionary_SchemaVersion_V1);
 *     bool isManifest = ccnxTlvDictionary_IsManifest(dict);
 *     // isManifest will be true
 * }
 * @endcode
 */
bool ccnxTlvDictionary_IsManifest(const CCNxTlvDictionary *dictionary);

/**
 * If in DEBUG mode, returns how long the message has been in the system
 *
 * If not in DEBUG mode, will always be {.tv_sec = 0, .tv_usec = 0}.  The time is based
 * on gettimeofday().
 *
 * Measured since the time when the dictionary was created
 *
 * @param [in] dictionary The dictionary instance whose lifetime is being queried
 *
 * @return struct timeval The lifetime of the dictionary message.
 *
 * Example:
 * @code
 * {
 *     ...
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     // do some things ...
 *     struct timeval = ccnxTlvDictionary_GetLifetime(dict);
 *     // use the time as needed
 * }
 * @endcode
 */
struct timeval ccnxTlvDictionary_GetLifetime(const CCNxTlvDictionary *dictionary);

/**
 * Display the dictionary and its contents
 *
 * The contents of the dictionary are printed to stdout. This is often useful
 * for verbose debugging purposes.
 *
 * @param [in] indent The number of tabs to indent all lines printed on stdout
 * @param [in] dictionary The dictionary instance whose contents will be printed to stdout
 *
 * Example:
 * @code
 * {
 *     ...
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_Display(dict, 0);
 * }
 * @endcode
 */
void ccnxTlvDictionary_Display(const CCNxTlvDictionary *dictionary, int indent);

/**
 * Determine if two CCNxTlvDictionary instances are equal.
 *
 * Two CCNxTlvDictionary instances are equal if, and only if, they contain the same number
 * of keys, the keys are equal, and each key points to the same value.
 *
 * The following equivalence relations on non-null `CCNxTlvDictionary` instances are maintained:
 *
 *  * It is reflexive: for any non-null reference value x, `ccnxTlvDictionary_Equals(x, x)`
 *      must return true.
 *
 *  * It is symmetric: for any non-null reference values x and y,
 *    `ccnxTlvDictionary_Equals(x, y)` must return true if and only if
 *        `ccnxTlvDictionary_Equals(y, x)` returns true.
 *
 *  * It is transitive: for any non-null reference values x, y, and z, if
 *        `ccnxTlvDictionary_Equals(x, y)` returns true and
 *        `ccnxTlvDictionary_Equals(y, z)` returns true,
 *        then  `ccnxTlvDictionary_Equals(x, z)` must return true.
 *
 *  * It is consistent: for any non-null reference values x and y, multiple
 *      invocations of `ccnxTlvDictionary_Equals(x, y)` consistently return true or
 *      consistently return false.
 *
 *  * For any non-null reference value x, `ccnxTlvDictionary_Equals(x, NULL)` must
 *      return false.
 *
 * @param a A pointer to a `CCNxTlvDictionary` instance.
 * @param b A pointer to a `CCNxTlvDictionary` instance.
 *
 * NULL == NULL, non-NULL != NULL, otherwise the dictionaries need to be the same
 * type, schema, and all fields must compare for equality.
 *
 * Equals does not include the CreationTime (lifetime) or the SetInfo() value.
 *
 * @return true Dictionaries are equal
 * @return false Dictionaries differ in some way
 *
 * Example:
 * @code
 * {
 *     ...
 *     CCNxTlvDictionary *dict1 = ccnxTlvDictionary_Create(5, 3);
 *     CCNxTlvDictionary *dict2 = ccnxTlvDictionary_Create(5, 3);
 *     if (ccnxTlvDictionary_Equals(dict1, dict2)) {
 *          // true
 *      } else {
 *          // false
 *      }
 * }
 * @endcode
 */
bool ccnxTlvDictionary_Equals(const CCNxTlvDictionary *a, const CCNxTlvDictionary *b);

/**
 * Allocates a new instance of the specified CCNxTlvDictionary that is
 * a "Shallow" copy of the original.  The new instance contains the
 * same contents as the original CCNxTlvDictionary but an Acquire()
 * operation is used where possible to form a new link to the original
 * contained content (a Copy() operation is used otherwise). Note that
 * this means that modifying the content of the copy will, in most
 * cases, modify the content of the original. In any case, the
 * contents are protected from premature deallocation.
 *
 * @param [in] source The dictionary to copy
 *
 * @return CCNxTlvDictionary A pointer to a copy of the source dictionary
 *
 * Example:
 * @code
 * {
 *     ...
 *     CCNxTlvDictionary *source = ccnxTlvDictionary_Create(5, 3);
 *     PARCBuffer *buffer = parcBuffer_Allocate(1);
 *     ccnxTlvDictionary_PutListBuffer(source, 1, 1, buffer);
 *     ...
 *     CCNxTlvDictionary *copy = ccnxTlvDictionary_ShallowCopy(source);
 *     PARCBuffer *buffcopy;
 *     uint32_t key = 0;
 *     ccnxTlvDictionary_ListGetByPosition(copy, 1, 0, &buffcopy, &key);
 *     ...
 *     assertTrue(ccnxTlvDictionary_Equals(source, copy), "Error: not a copy");
 *     assertTrue(buffcopy == buffer, "Error: not a shallow copy");
 * }
 * @endcode
 */
CCNxTlvDictionary *ccnxTlvDictionary_ShallowCopy(const CCNxTlvDictionary *source);

/**
 * Set the pointer to the implementation used to create the message type represented by this
 * CCNxTlvDictionary. For example, if the CCNxTlvDictionary represents a V1 ContentObject,
 * the implementation pointer should be set to &CCNxContentObjectFacadeV1_Implementation.
 * The type can be inferred from the dictionary's schemaVersion and messageType.
 *
 * Consumers of this CCNxTlvDictionary should use the implementation pointer to access
 * fields in the dictionary. Examples of implementations would be those that define
 * {@link CCNxContentObjectInterface} or {@link CCNxInterestInterface}, such as
 * {@link CCNxContentObjectFacadeV1_Implementation} or {CCNxInterestFacadeV1_Implementation}.
 *
 * @param [in] dictionary The dictionary instance on which to set the implementation pointer.
 * @param [in] implementation The address of the implementation to be used to access this dictionary.
 *
 * Example:
 * @code
 * {
 *     ...
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_SetMessageInterface(&CCNxContentObjectFacadeV1_Implementation);
 * }
 * @endcode
 * @see `ccnxTlvDictionary_GetMessageTypeInterface`
 */
void ccnxTlvDictionary_SetMessageInterface(CCNxTlvDictionary *dictionary, const CCNxMessageInterface *implementation);

/**
 * Return the address of the implementation used to access fields in the the message represented by this
 * CCNxTlvDictionary instance. The implementation pointer would typically be
 * {@link CCNxContentObjectFacadeV1_Implementation} or {@link CCNxContentObjectFacadeV1_Implementation}.
 * If it is not set, it can be inferred from the messageType and the schemaVersion.
 *
 * @param [in] dictionary The dictionary instance from which to retrieve the implementation pointer.
 *
 * Example:
 * @code
 * {
 *     ...
 *     CCNxTlvDictionary *dict = ccnxTlvDictionary_Create(5, 3);
 *     ccnxTlvDictionary_SetMessageTypeInterface(&CCNxContentObjectFacadeV1_Implementation);
 *     CCNxContentObjectInterface *impl = ccnxTlvDictionary_GetMessageTypeInterface(dict);
 * }
 * @endcode
 * @see `ccnxTlvDictionary_SetMessageTypeInterface`
 * @see `ccnxContentObjectInterface_GetInterface`
 * @see `ccnxInterestInterface_GetInterface`
 */
CCNxMessageInterface *ccnxTlvDictionary_GetMessageInterface(const CCNxTlvDictionary *dictionary);
#endif // libccnx_ccnx_TlvDictionary_h
