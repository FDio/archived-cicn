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

#include <parc/assert/parc_Assert.h>

#include <string.h>

#include <parc/algol/parc_Dictionary.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_TreeRedBlack.h>

struct parc_dictionary {
    PARCDictionary_CompareKey keyCompareFunction;
    PARCDictionary_KeyHashFunc keyHashFunction;
    PARCDictionary_FreeKey keyFreeFunction;
    PARCDictionary_FreeValue valueFreeFunction;
    PARCDictionary_ValueEquals valueEqualsFunction;
    PARCTreeRedBlack *tree;
};


PARCDictionary *
parcDictionary_Create(PARCDictionary_CompareKey keyCompareFunction,
                      PARCDictionary_KeyHashFunc keyHashFunction,
                      PARCDictionary_FreeKey keyFreeFunction,
                      PARCDictionary_ValueEquals valueEqualsFunction,
                      PARCDictionary_FreeValue valueFreeFunction)
{
    parcAssertNotNull(keyCompareFunction, "KeyCompareFunction can't be null");
    parcAssertNotNull(keyHashFunction, "KeyHashFunction can't be null");
    PARCDictionary *dictionary = parcMemory_Allocate(sizeof(PARCDictionary));
    parcAssertNotNull(dictionary, "parcMemory_Allocate(%zu) returned NULL", sizeof(PARCDictionary));
    dictionary->keyCompareFunction = keyCompareFunction;
    dictionary->keyHashFunction = keyHashFunction;
    dictionary->keyFreeFunction = keyFreeFunction;
    dictionary->valueFreeFunction = valueFreeFunction;
    dictionary->valueEqualsFunction = valueEqualsFunction;
    dictionary->tree = parcTreeRedBlack_Create(keyCompareFunction,
                                               keyFreeFunction,
                                               NULL,
                                               valueEqualsFunction,
                                               valueFreeFunction,
                                               NULL);
    return dictionary;
}


void
parcDictionary_Destroy(PARCDictionary **dictionaryPointer)
{
    parcAssertNotNull(dictionaryPointer, "Pointer to dictionary pointer can't be NULL");
    parcAssertNotNull(*dictionaryPointer, "Pointer to dictionary can't be NULL");
    parcTreeRedBlack_Destroy(&((*dictionaryPointer)->tree));
    parcMemory_Deallocate((void **) dictionaryPointer);
    *dictionaryPointer = NULL;
}

void
parcDictionary_SetValue(PARCDictionary *dictionary, void *key, void *value)
{
    parcAssertNotNull(dictionary, "dictionary pointer can't be NULL");
    parcAssertNotNull(key, "Key pointer can't be NULL");
    parcTreeRedBlack_Insert(dictionary->tree, key, value);
}

void *
parcDictionary_GetValue(PARCDictionary *dictionary, const void *key)
{
    parcAssertNotNull(dictionary, "dictionary pointer can't be NULL");
    parcAssertNotNull(key, "Key pointer can't be NULL");
    return parcTreeRedBlack_Get(dictionary->tree, key);
}

void *
parcDictionary_RemoveValue(PARCDictionary *dictionary, const void *key)
{
    parcAssertNotNull(dictionary, "dictionary pointer can't be NULL");
    parcAssertNotNull(key, "Key pointer can't be NULL");
    return parcTreeRedBlack_Remove(dictionary->tree, key);
}

void
parcDictionary_RemoveAndDestroyValue(PARCDictionary *dictionary, const void *key)
{
    parcAssertNotNull(dictionary, "dictionary pointer can't be NULL");
    parcAssertNotNull(key, "Key pointer can't be NULL");
    parcTreeRedBlack_RemoveAndDestroy(dictionary->tree, key);
}

PARCArrayList *
parcDictionary_Keys(const PARCDictionary *dictionary)
{
    parcAssertNotNull(dictionary, "dictionary pointer can't be NULL");
    return parcTreeRedBlack_Keys(dictionary->tree);
}

PARCArrayList *
parcDictionary_Values(const PARCDictionary *dictionary)
{
    parcAssertNotNull(dictionary, "dictionary pointer can't be NULL");
    return parcTreeRedBlack_Values(dictionary->tree);
}

size_t
parcDictionary_Size(const PARCDictionary *dictionary)
{
    parcAssertNotNull(dictionary, "dictionary pointer can't be NULL");
    return parcTreeRedBlack_Size(dictionary->tree);
}

int
parcDictionary_Equals(const PARCDictionary *dictionary1, const PARCDictionary *dictionary2)
{
    parcAssertNotNull(dictionary1, "dictionary pointer can't be NULL");
    parcAssertNotNull(dictionary2, "dictionary pointer can't be NULL");
    return parcTreeRedBlack_Equals(dictionary1->tree, dictionary2->tree);
}
