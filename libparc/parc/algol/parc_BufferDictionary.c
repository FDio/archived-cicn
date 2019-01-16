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
 * The dictionary is implemented with the parc_HashCodeTable backend.  This implementation
 * is inefficient for additions with duplicate keys, because the semantics of parc_HashCodeTable
 * are not the same as parc_BufferDictionary in returning values for Put and Remove.
 *
 */
#include <config.h>

#include <parc/assert/parc_Assert.h>

#include <parc/algol/parc_BufferDictionary.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_HashCodeTable.h>

struct parc_buffer_dictionary {
    PARCHashCodeTable *hashtable;
};

// Wrapper to go from void * to PARCBuffer *
static bool
_parcBufferEquals(const void *a, const void *b)
{
    return parcBuffer_Equals((const PARCBuffer *) a, (const PARCBuffer *) b);
}

// Wrapper to go from void * to PARCBuffer *
static PARCHashCode
_parcBufferHashCode(const void *a)
{
    return parcBuffer_HashCode((const PARCBuffer *) a);
}

// Wrapper to go from void ** to PARCBuffer **
static void
_parcBufferRelease(void **bufferVoidPtr)
{
    parcBuffer_Release((PARCBuffer **) bufferVoidPtr);
}

/*
 * Initialise a parcBufferDictionary instance.
 * @return The same pointer as `result`.
 */
static PARCBufferDictionary *
_init(PARCBufferDictionary *result)
{
    result->hashtable = parcHashCodeTable_Create(_parcBufferEquals, _parcBufferHashCode, _parcBufferRelease, _parcBufferRelease);
    return result;
}

/**
 * Cleans up the internal memory of a PARCBufferDictionary
 *
 * @param [in] dictionaryPtr Double pointer to the dictionary to finalize
 */
static void
_destroy(PARCBufferDictionary **dictionaryPtr)
{
    parcAssertNotNull(dictionaryPtr, "Double pointer dictionaryPtr must be non-NULL");
    parcAssertNotNull(*dictionaryPtr, "Double pointer dictionaryPtr must dereference to non-NULL");

    PARCBufferDictionary *dict = *dictionaryPtr;

    parcHashCodeTable_Destroy(&dict->hashtable);
}


parcObject_ExtendPARCObject(PARCBufferDictionary, _destroy, NULL, NULL, NULL, NULL, NULL, NULL);

PARCBufferDictionary *
parcBufferDictionary_Create(void)
{
    PARCBufferDictionary *result = parcObject_CreateInstance(PARCBufferDictionary);
    if (result != NULL) {
        return _init(result);
    }

    return NULL;
}

parcObject_ImplementAcquire(parcBufferDictionary, PARCBufferDictionary);

parcObject_ImplementRelease(parcBufferDictionary, PARCBufferDictionary);

PARCBuffer *
parcBufferDictionary_Put(PARCBufferDictionary *dictionary, PARCBuffer *key, PARCBuffer *value)
{
    parcAssertNotNull(dictionary, "Parameter dictionary must be non-NULL");
    parcAssertNotNull(key, "Parameter key must be non-NULL");
    parcAssertNotNull(value, "Parameter value must be non-NULL");

    PARCBuffer *oldValue = NULL;

    // We use reference counted copyies of the supplied parameters
    PARCBuffer *key_copy = parcBuffer_Acquire(key);
    PARCBuffer *value_copy = parcBuffer_Acquire(value);

    if (!parcHashCodeTable_Add(dictionary->hashtable, key_copy, value_copy)) {
        // parcHashCodeTable_Del will free the referece, to make a copy of it
        PARCBuffer *original = (PARCBuffer *) parcHashCodeTable_Get(dictionary->hashtable, key_copy);
        oldValue = parcBuffer_Acquire(original);
        parcHashCodeTable_Del(dictionary->hashtable, key_copy);
        parcHashCodeTable_Add(dictionary->hashtable, key_copy, value_copy);
    }

    return oldValue;
}

PARCBuffer *
parcBufferDictionary_Get(PARCBufferDictionary *dictionary, PARCBuffer *key)
{
    parcAssertNotNull(dictionary, "Parameter dictionary must be non-NULL");
    parcAssertNotNull(key, "Parameter key must be non-NULL");

    return parcHashCodeTable_Get(dictionary->hashtable, key);
}

PARCBuffer *
parcBufferDictionary_Remove(PARCBufferDictionary *dictionary, PARCBuffer *key)
{
    parcAssertNotNull(dictionary, "Parameter dictionary must be non-NULL");
    parcAssertNotNull(key, "Parameter key must be non-NULL");

    // parcHashCodeTable_Del will free the referece, to make a copy of it
    PARCBuffer *original = (PARCBuffer *) parcHashCodeTable_Get(dictionary->hashtable, key);
    PARCBuffer *oldValue = parcBuffer_Acquire(original);
    parcHashCodeTable_Del(dictionary->hashtable, key);
    return oldValue;
}
