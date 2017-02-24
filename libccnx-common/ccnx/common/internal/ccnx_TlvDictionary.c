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
 *
 */

#include <config.h>
#include <stdlib.h>
#include <sys/time.h>
#include <inttypes.h>
#include <stdio.h>

#include <ccnx/common/ccnx_Name.h>

#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_JSON.h>

#define DEBUG_ALLOCS 0

struct ccnx_tlv_dictionary_entry;
typedef struct ccnx_tlv_list_entry _CCNxTlvDictionaryListEntry;

typedef enum {
    CCNxTlvDictionaryType_Unknown,
    CCNxTlvDictionaryType_Interest,
    CCNxTlvDictionaryType_ContentObject,
    CCNxTlvDictionaryType_Control,
    CCNxTlvDictionaryType_InterestReturn,
    CCNxTlvDictionaryType_Manifest
} _CCNxTlvDictionaryType;

// These form a singly linked list
struct ccnx_tlv_list_entry {
    _CCNxTlvDictionaryListEntry *next;
    PARCBuffer *buffer;
    uint16_t key;
};

#define ENTRY_UNSET   ((int) 0)
#define ENTRY_BUFFER  ((int) 1)
#define ENTRY_NAME    ((int) 2)
#define ENTRY_INTEGER ((int) 3)
#define ENTRY_IOVEC   ((int) 4)
#define ENTRY_JSON    ((int) 5)
#define ENTRY_OBJECT  ((int) 6)

static struct dictionary_type_string {
    _CCNxTlvDictionaryType type;
    const char *string;
} ccnxTlvDictionaryTypeStrings [] = {
    { .type = CCNxTlvDictionaryType_Unknown,        .string = "Invalid"        },
    { .type = CCNxTlvDictionaryType_Interest,       .string = "Interest"       },
    { .type = CCNxTlvDictionaryType_ContentObject,  .string = "Content Object" },
    { .type = CCNxTlvDictionaryType_Control,        .string = "Control"        },
    { .type = CCNxTlvDictionaryType_InterestReturn, .string = "InterestReturn" },
    { .type = CCNxTlvDictionaryType_Manifest,       .string = "Manifest"       },
    { .type = UINT32_MAX,                           .string = NULL             },
};

static struct dictionary_entry_type_string {
    int type;
    const char *string;
} ccnxTlvDictionaryEntryTypeStrings [] = {
    { .type = ENTRY_UNSET,   .string = "Unset"   },
    { .type = ENTRY_BUFFER,  .string = "Buffer"  },
    { .type = ENTRY_NAME,    .string = "Name"    },
    { .type = ENTRY_INTEGER, .string = "Integer" },
    { .type = ENTRY_IOVEC,   .string = "IoVec"   },
    { .type = ENTRY_JSON,    .string = "JSON"    },
    { .type = ENTRY_OBJECT,  .string = "Object"  },
    { .type = UINT32_MAX,    .string = NULL      },
};

static const char *ccnxTlvDictionaryTypeUnknown = "Unknown";

static const char *
_ccnxTlvDictionaryEntryTypeToString(int entryType)
{
    for (int i = 0; ccnxTlvDictionaryEntryTypeStrings[i].string != NULL; i++) {
        if (ccnxTlvDictionaryEntryTypeStrings[i].type == entryType) {
            return ccnxTlvDictionaryEntryTypeStrings[i].string;
        }
    }
    return ccnxTlvDictionaryTypeUnknown;
}

static const char *
_ccnxTlvDictionaryTypeToString(_CCNxTlvDictionaryType dictionaryType)
{
    for (int i = 0; ccnxTlvDictionaryTypeStrings[i].string != NULL; i++) {
        if (ccnxTlvDictionaryTypeStrings[i].type == dictionaryType) {
            return ccnxTlvDictionaryTypeStrings[i].string;
        }
    }
    return ccnxTlvDictionaryTypeUnknown;
}


typedef struct ccnx_tlv_dictionary_entry {
    int entryType;
    union u_entry {
        PARCBuffer *buffer;
        uint64_t integer;
        CCNxName   *name;
        CCNxCodecNetworkBufferIoVec *vec;
        PARCJSON   *json;
        PARCObject *object;
    } _entry;
} _CCNxTlvDictionaryEntry;

struct ccnx_tlv_dictionary {
#define FIXED_LIST_LENGTH 8
    // These are linked lists where we put unknown TLV types.  This one static allocation should
    // be enough for all the current packet formats.
    _CCNxTlvDictionaryListEntry *fixedListHeads[FIXED_LIST_LENGTH];

    // if we need to allocate beyond FIXED_LIST_LENGTH, put them here
    _CCNxTlvDictionaryListEntry **extraListHeads;

    size_t fastArraySize;
    size_t listSize;

    _CCNxTlvDictionaryType dictionaryType;
    CCNxTlvDictionary_SchemaVersion schemaVersion;

    // Detects changes in the dictionary that were not caused by us
    uint32_t generation;

    struct timeval creationTime;

    void (*infoFreeFunction)(void **infoPtr);
    void *info;

    // A pointer to the implementation functions for the type contained by this dictionary.
    // It's a runtime static, and is not encoded. Thus, when a dictionary is received over
    // the wire, it will need to be initialized based on the dictionaryType and schemaVersion.
    CCNxMessageInterface *messageInterface;

    // will be allocated as part of the ccnx_tlv_dictionary
    _CCNxTlvDictionaryEntry directArray[CCNxCodecSchemaV1TlvDictionary_MessageFastArray_END];
};

static _CCNxTlvDictionaryListEntry *
_ccnxTlvDictionaryListEntry_Create(uint32_t key, const PARCBuffer *buffer)
{
    _CCNxTlvDictionaryListEntry *entry = parcMemory_AllocateAndClear(sizeof(_CCNxTlvDictionaryListEntry));
    assertNotNull(entry, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(_CCNxTlvDictionaryListEntry));
    entry->key = key;
    entry->buffer = parcBuffer_Acquire(buffer);

    return entry;
}

static void
_ccnxTlvDictionaryListEntry_Release(_CCNxTlvDictionaryListEntry **entryPtr)
{
    _CCNxTlvDictionaryListEntry *entry = *entryPtr;
    parcBuffer_Release(&entry->buffer);
    parcMemory_Deallocate((void **) &entry);
    *entryPtr = NULL;
}

static void
_ccnxTlvDictionaryEntry_ListRelease(_CCNxTlvDictionaryListEntry **listHeadPtr)
{
    _CCNxTlvDictionaryListEntry *listHead = *listHeadPtr;
    while (listHead) {
        _CCNxTlvDictionaryListEntry *next = listHead->next;
        _ccnxTlvDictionaryListEntry_Release(&listHead);
        listHead = next;
    }
    *listHeadPtr = NULL;
}

static void
_ccnxTlvDictionary_FinalRelease(CCNxTlvDictionary **dictionaryPtr)
{
    CCNxTlvDictionary *dictionary = *dictionaryPtr;

    // release any entries stored in the fast array
    for (int i = 0; i < dictionary->fastArraySize; i++) {
        switch (dictionary->directArray[i].entryType) {
            case ENTRY_BUFFER:
                parcBuffer_Release(&dictionary->directArray[i]._entry.buffer);
                break;
            case ENTRY_NAME:
                ccnxName_Release(&dictionary->directArray[i]._entry.name);
                break;
            case ENTRY_IOVEC:
                ccnxCodecNetworkBufferIoVec_Release(&dictionary->directArray[i]._entry.vec);
                break;
            case ENTRY_JSON:
                parcJSON_Release(&dictionary->directArray[i]._entry.json);
                break;
            case ENTRY_OBJECT:
                parcObject_Release(&dictionary->directArray[i]._entry.object);
                break;
            default:
                // other types are direct storage
                break;
        }
    }

    for (int i = 0; i < FIXED_LIST_LENGTH; i++) {
        if (dictionary->fixedListHeads[i]) {
            _ccnxTlvDictionaryEntry_ListRelease(&dictionary->fixedListHeads[i]);
        }
    }

    if (dictionary->extraListHeads) {
        for (int i = FIXED_LIST_LENGTH; i < dictionary->listSize; i++) {
            if (dictionary->extraListHeads[i - FIXED_LIST_LENGTH]) {
                _ccnxTlvDictionaryEntry_ListRelease(&dictionary->extraListHeads[i - FIXED_LIST_LENGTH]);
            }
        }
        parcMemory_Deallocate((void **) &(dictionary->extraListHeads));
    }

    if (dictionary->infoFreeFunction) {
        dictionary->infoFreeFunction(&dictionary->info);
    }

#if DEBUG_ALLOCS
    printf("finalize dictionary %p (final)\n", dictionary);
#endif
}

parcObject_ExtendPARCObject(CCNxTlvDictionary, _ccnxTlvDictionary_FinalRelease,
                            NULL, NULL, ccnxTlvDictionary_Equals, NULL, NULL, NULL);

parcObject_ImplementAcquire(ccnxTlvDictionary, CCNxTlvDictionary);

parcObject_ImplementRelease(ccnxTlvDictionary, CCNxTlvDictionary);

static void
_ccnxTlvDictionary_GetTimeOfDay(struct timeval *outputTime)
{
#ifdef DEBUG
    // if in debug mode, time messages
    gettimeofday(outputTime, NULL);
#else
    *outputTime = (struct timeval) { 0, 0 };
#endif
}


CCNxTlvDictionary *
ccnxTlvDictionary_Create(size_t bufferCount, size_t listCount)
{
    CCNxTlvDictionary *dictionary = (CCNxTlvDictionary *) parcObject_CreateAndClearInstance(CCNxTlvDictionary);

    if (dictionary != NULL) {
        _ccnxTlvDictionary_GetTimeOfDay(&dictionary->creationTime);

        dictionary->dictionaryType = CCNxTlvDictionaryType_Unknown;
        dictionary->fastArraySize = bufferCount;
        dictionary->listSize = listCount;

        dictionary->infoFreeFunction = NULL;
        dictionary->info = NULL;

        dictionary->extraListHeads = NULL;
        // dictionary->directArray is allocated as part of parcObject
    }

#if DEBUG_ALLOCS
    printf("allocate dictionary %p\n", dictionary);
#endif

    return dictionary;
}

CCNxTlvDictionary *
ccnxTlvDictionary_ShallowCopy(const CCNxTlvDictionary *source)
{
    size_t bufferCount = source->fastArraySize;
    size_t listCount = source->listSize;
    CCNxTlvDictionary  *newDictionary = ccnxTlvDictionary_Create(bufferCount, listCount);

    if (newDictionary != NULL) {
        newDictionary->dictionaryType = source->dictionaryType;
        newDictionary->schemaVersion = source->schemaVersion;
        newDictionary->generation = source->generation;
        newDictionary->creationTime = source->creationTime;
        newDictionary->messageInterface = source->messageInterface;
        newDictionary->info = source->info;
        newDictionary->infoFreeFunction = source->infoFreeFunction;

        // Update listHeads
        for (uint32_t key = 0; key < source->listSize; ++key) {
            size_t listSize = ccnxTlvDictionary_ListSize(source, key);
            for (size_t i = 0; i < listSize; ++i) {
                PARCBuffer *buffer;
                uint32_t bKey;
                ccnxTlvDictionary_ListGetByPosition(source, key, i, &buffer, &bKey);
                parcBuffer_Acquire(buffer);
                ccnxTlvDictionary_PutListBuffer(newDictionary, key, bKey, buffer);
                parcBuffer_Release(&buffer);
            }
        }

        // Update directArray
        for (uint32_t key = 0; key < source->fastArraySize; ++key) {
            switch (source->directArray[key].entryType) {
                case ENTRY_BUFFER:
                    ccnxTlvDictionary_PutBuffer(newDictionary, key, ccnxTlvDictionary_GetBuffer(source, key));
                    break;
                case ENTRY_NAME:
                    ccnxTlvDictionary_PutName(newDictionary, key, ccnxTlvDictionary_GetName(source, key));
                    break;
                case ENTRY_IOVEC:
                    ccnxTlvDictionary_PutIoVec(newDictionary, key, ccnxTlvDictionary_GetIoVec(source, key));
                    break;
                case ENTRY_JSON:
                    ccnxTlvDictionary_PutJson(newDictionary, key, ccnxTlvDictionary_GetJson(source, key));
                    break;
                case ENTRY_INTEGER:
                    ccnxTlvDictionary_PutInteger(newDictionary, key, ccnxTlvDictionary_GetInteger(source, key));
                    break;
                case ENTRY_OBJECT:
                    ccnxTlvDictionary_PutObject(newDictionary, key, ccnxTlvDictionary_GetObject(source, key));
                    break;
                default:
                    break;
            }
        }
    }

    return newDictionary;
}

bool
ccnxTlvDictionary_PutBuffer(CCNxTlvDictionary *dictionary, uint32_t key, const PARCBuffer *buffer)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertNotNull(buffer, "Parameter buffer must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);

    if (dictionary->directArray[key].entryType == ENTRY_UNSET) {
        dictionary->directArray[key].entryType = ENTRY_BUFFER;
        dictionary->directArray[key]._entry.buffer = parcBuffer_Acquire(buffer);
        return true;
    }
    return false;
}

bool
ccnxTlvDictionary_PutObject(CCNxTlvDictionary *dictionary, uint32_t key, const PARCObject *object)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertNotNull(object, "Parameter object must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key %ud must be less than %zu", key, dictionary->fastArraySize);

    if (dictionary->directArray[key].entryType == ENTRY_UNSET) {
        dictionary->directArray[key].entryType = ENTRY_OBJECT;
        dictionary->directArray[key]._entry.object = parcObject_Acquire(object);
        return true;
    }
    return false;
}

bool
ccnxTlvDictionary_PutName(CCNxTlvDictionary *dictionary, uint32_t key, const CCNxName *name)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertNotNull(name, "Parameter buffer must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);

    if (dictionary->directArray[key].entryType == ENTRY_UNSET) {
        dictionary->directArray[key].entryType = ENTRY_NAME;
        dictionary->directArray[key]._entry.name = ccnxName_Acquire(name);
        return true;
    }
    return false;
}

bool
ccnxTlvDictionary_PutInteger(CCNxTlvDictionary *dictionary, uint32_t key, const uint64_t value)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);

    if (dictionary->directArray[key].entryType == ENTRY_UNSET || dictionary->directArray[key].entryType == ENTRY_INTEGER) {
        dictionary->directArray[key].entryType = ENTRY_INTEGER;
        dictionary->directArray[key]._entry.integer = value;
        return true;
    }
    return false;
}

bool
ccnxTlvDictionary_PutIoVec(CCNxTlvDictionary *dictionary, uint32_t key, const CCNxCodecNetworkBufferIoVec *vec)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertNotNull(vec, "Parameter buffer must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);

    if (dictionary->directArray[key].entryType == ENTRY_UNSET) {
        dictionary->directArray[key].entryType = ENTRY_IOVEC;
        dictionary->directArray[key]._entry.vec = ccnxCodecNetworkBufferIoVec_Acquire((CCNxCodecNetworkBufferIoVec *) vec);
        return true;
    }
    return false;
}

bool
ccnxTlvDictionary_PutJson(CCNxTlvDictionary *dictionary, uint32_t key, const PARCJSON *json)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertNotNull(json, "Parameter json must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);

    if (dictionary->directArray[key].entryType == ENTRY_UNSET) {
        dictionary->directArray[key].entryType = ENTRY_JSON;
        dictionary->directArray[key]._entry.json = parcJSON_Acquire(json);
        return true;
    }
    return false;
}

CCNxCodecNetworkBufferIoVec *
ccnxTlvDictionary_GetIoVec(const CCNxTlvDictionary *dictionary, uint32_t key)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);

    if (dictionary->directArray[key].entryType == ENTRY_IOVEC) {
        return dictionary->directArray[key]._entry.vec;
    }
    return NULL;
}

// If you need to change the list head, use this
static _CCNxTlvDictionaryListEntry **
_getListHeadReference(CCNxTlvDictionary *dictionary, uint32_t listKey)
{
    if (listKey < FIXED_LIST_LENGTH) {
        return &dictionary->fixedListHeads[listKey];
    } else {
        if (dictionary->extraListHeads == NULL) {
            dictionary->extraListHeads = parcMemory_AllocateAndClear(sizeof(_CCNxTlvDictionaryListEntry *) * (dictionary->listSize - FIXED_LIST_LENGTH));
        }

        return &dictionary->extraListHeads[listKey - FIXED_LIST_LENGTH];
    }
}

// If not going to modify the list, use this
static _CCNxTlvDictionaryListEntry *
_getListHead(const CCNxTlvDictionary *dictionary, uint32_t listKey)
{
    _CCNxTlvDictionaryListEntry **head = _getListHeadReference((CCNxTlvDictionary *) dictionary, listKey);
    return *head;
}

bool
ccnxTlvDictionary_PutListBuffer(CCNxTlvDictionary *dictionary, uint32_t listKey, uint32_t key, const PARCBuffer *buffer)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertNotNull(buffer, "Parameter buffer must be non-null");
    assertTrue(listKey < dictionary->listSize, "Parameter key must be less than %zu", dictionary->listSize);

    _CCNxTlvDictionaryListEntry *entry = _ccnxTlvDictionaryListEntry_Create(key, buffer);

    _CCNxTlvDictionaryListEntry **head = _getListHeadReference(dictionary, listKey);
    if (*head) {
        // insert new value at list head
        entry->next = *head;
        *head = entry;
    } else {
        // new value is the list head
        *head = entry;
    }
    return true;
}

bool
ccnxTlvDictionary_IsValueBuffer(const CCNxTlvDictionary *dictionary, uint32_t key)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);
    return (dictionary->directArray[key].entryType == ENTRY_BUFFER);
}

bool
ccnxTlvDictionary_IsValueObject(const CCNxTlvDictionary *dictionary, uint32_t key)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);
    return (dictionary->directArray[key].entryType == ENTRY_OBJECT);
}

bool
ccnxTlvDictionary_IsValueInteger(const CCNxTlvDictionary *dictionary, uint32_t key)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);
    return (dictionary->directArray[key].entryType == ENTRY_INTEGER);
}

bool
ccnxTlvDictionary_IsValueName(const CCNxTlvDictionary *dictionary, uint32_t key)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);
    return (dictionary->directArray[key].entryType == ENTRY_NAME);
}

bool
ccnxTlvDictionary_IsValueIoVec(const CCNxTlvDictionary *dictionary, uint32_t key)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);
    return (dictionary->directArray[key].entryType == ENTRY_IOVEC);
}

bool
ccnxTlvDictionary_IsValueJson(const CCNxTlvDictionary *dictionary, uint32_t key)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);
    return (dictionary->directArray[key].entryType == ENTRY_JSON);
}

PARCBuffer *
ccnxTlvDictionary_GetBuffer(const CCNxTlvDictionary *dictionary, uint32_t key)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);

    // For now return NULL for backward compatability with prior code, case 1011
    if (dictionary->directArray[key].entryType == ENTRY_BUFFER) {
        return dictionary->directArray[key]._entry.buffer;
    }
    return NULL;
}

CCNxName *
ccnxTlvDictionary_GetName(const CCNxTlvDictionary *dictionary, uint32_t key)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);

    if (dictionary->directArray[key].entryType == ENTRY_NAME) {
        return dictionary->directArray[key]._entry.name;
    }
    return NULL;
}

uint64_t
ccnxTlvDictionary_GetInteger(const CCNxTlvDictionary *dictionary, uint32_t key)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);

    trapIllegalValueIf(dictionary->directArray[key].entryType != ENTRY_INTEGER,
                       "Key %u is of type %d",
                       key, dictionary->directArray[key].entryType)
    {
        ccnxTlvDictionary_Display(dictionary, 3);
    }

    return dictionary->directArray[key]._entry.integer;
}


PARCJSON *
ccnxTlvDictionary_GetJson(const CCNxTlvDictionary *dictionary, uint32_t key)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);

    if (dictionary->directArray[key].entryType == ENTRY_JSON) {
        return dictionary->directArray[key]._entry.json;
    }
    return NULL;
}

PARCObject *
ccnxTlvDictionary_GetObject(const CCNxTlvDictionary *dictionary, uint32_t key)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertTrue(key < dictionary->fastArraySize, "Parameter key must be less than %zu", dictionary->fastArraySize);

    if (dictionary->directArray[key].entryType == ENTRY_OBJECT) {
        return dictionary->directArray[key]._entry.object;
    }

    return NULL;
}

bool
ccnxTlvDictionary_ListGetByPosition(const CCNxTlvDictionary *dictionary, uint32_t listKey, size_t listPosition, PARCBuffer **bufferPtr, uint32_t *keyPtr)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertNotNull(bufferPtr, "Parameter bufferPtr must be non-null");
    assertNotNull(keyPtr, "Parameter keyPtr must be non-null");
    assertTrue(listKey < dictionary->listSize, "Parameter key must be less than %zu", dictionary->listSize);

    _CCNxTlvDictionaryListEntry *entry = _getListHead(dictionary, listKey);
    while (entry) {
        if (listPosition == 0) {
            *bufferPtr = entry->buffer;
            *keyPtr = entry->key;
            return true;
        }
        entry = entry->next;
        --listPosition;
    }

    return false;
}


PARCBuffer *
ccnxTlvDictionary_ListGetByType(const CCNxTlvDictionary *dictionary, uint32_t listKey, uint32_t type)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertTrue(listKey < dictionary->listSize, "Parameter key must be less than %zu", dictionary->listSize);

    PARCBuffer *buffer = NULL;
    _CCNxTlvDictionaryListEntry *entry = _getListHead(dictionary, listKey);
    while (entry) {
        if (entry->key == type) {
            buffer = entry->buffer;
            break;
        }
        entry = entry->next;
    }

    return buffer;
}


size_t
ccnxTlvDictionary_ListSize(const CCNxTlvDictionary *dictionary, uint32_t listKey)
{
    assertNotNull(dictionary, "Parameter dictionary must be non-null");
    assertTrue(listKey < dictionary->listSize, "Parameter key must be less than %zu", dictionary->listSize);

    size_t size = 0;
    _CCNxTlvDictionaryListEntry *entry = _getListHead(dictionary, listKey);
    while (entry) {
        size++;
        entry = entry->next;
    }

    return size;
}

void
ccnxTlvDictionary_SetMessageType_Interest(CCNxTlvDictionary *dictionary, CCNxTlvDictionary_SchemaVersion schemaVersion)
{
    dictionary->dictionaryType = CCNxTlvDictionaryType_Interest;
    dictionary->schemaVersion = schemaVersion;
}

void
ccnxTlvDictionary_SetMessageType_ContentObject(CCNxTlvDictionary *dictionary, CCNxTlvDictionary_SchemaVersion schemaVersion)
{
    dictionary->dictionaryType = CCNxTlvDictionaryType_ContentObject;
    dictionary->schemaVersion = schemaVersion;
}

void
ccnxTlvDictionary_SetMessageType_Control(CCNxTlvDictionary *dictionary, CCNxTlvDictionary_SchemaVersion schemaVersion)
{
    dictionary->dictionaryType = CCNxTlvDictionaryType_Control;
    dictionary->schemaVersion = schemaVersion;
}

void
ccnxTlvDictionary_SetMessageType_InterestReturn(CCNxTlvDictionary *dictionary, CCNxTlvDictionary_SchemaVersion schemaVersion)
{
    dictionary->dictionaryType = CCNxTlvDictionaryType_InterestReturn;
    dictionary->schemaVersion = schemaVersion;
}

void
ccnxTlvDictionary_SetMessageType_Manifest(CCNxTlvDictionary *dictionary, CCNxTlvDictionary_SchemaVersion schemaVersion)
{
    dictionary->dictionaryType = CCNxTlvDictionaryType_Manifest;
    dictionary->schemaVersion = schemaVersion;
}

bool
ccnxTlvDictionary_IsInterest(const CCNxTlvDictionary *dictionary)
{
    return (dictionary->dictionaryType == CCNxTlvDictionaryType_Interest);
}

bool
ccnxTlvDictionary_IsInterestReturn(const CCNxTlvDictionary *dictionary)
{
    return (dictionary->dictionaryType == CCNxTlvDictionaryType_InterestReturn);
}

bool
ccnxTlvDictionary_IsContentObject(const CCNxTlvDictionary *dictionary)
{
    return (dictionary->dictionaryType == CCNxTlvDictionaryType_ContentObject);
}

bool
ccnxTlvDictionary_IsControl(const CCNxTlvDictionary *dictionary)
{
    return (dictionary->dictionaryType == CCNxTlvDictionaryType_Control);
}

bool
ccnxTlvDictionary_IsManifest(const CCNxTlvDictionary *dictionary)
{
    return (dictionary->dictionaryType == CCNxTlvDictionaryType_Manifest);
}

CCNxTlvDictionary_SchemaVersion
ccnxTlvDictionary_GetSchemaVersion(const CCNxTlvDictionary *dictionary)
{
    return dictionary->schemaVersion;
}

void
ccnxTlvDictionary_SetMessageInterface(CCNxTlvDictionary *dictionary, const CCNxMessageInterface *implementation)
{
    dictionary->messageInterface = (CCNxMessageInterface *) implementation;
}

CCNxMessageInterface *
ccnxTlvDictionary_GetMessageInterface(const CCNxTlvDictionary *dictionary)
{
    return dictionary->messageInterface;
}

struct timeval
ccnxTlvDictionary_GetLifetime(const CCNxTlvDictionary *dictionary)
{
    struct timeval now;
    _ccnxTlvDictionary_GetTimeOfDay(&now);
    timersub(&now, &dictionary->creationTime, &now);
    return now;
}

static void
_ccnxTlvDictionary_DisplayBuffer(const _CCNxTlvDictionaryEntry *entry, int index)
{
    printf("     Entry %3d type %8s pointer %p\n", index, _ccnxTlvDictionaryEntryTypeToString(entry->entryType), (void *) entry->_entry.buffer);
    parcBuffer_Display(entry->_entry.buffer, 6);
}

static void
_ccnxTlvDictionary_DisplayInteger(const _CCNxTlvDictionaryEntry *entry, int index)
{
    printf("     Entry %3d type %8s value 0x%" PRIX64 " (%" PRIu64 ")\n", index, _ccnxTlvDictionaryEntryTypeToString(entry->entryType), entry->_entry.integer, entry->_entry.integer);
}

static void
_ccnxTlvDictionary_DisplayIoVec(const _CCNxTlvDictionaryEntry *entry, int index)
{
    printf("     Entry %3d type %8s pointer %p\n", index, _ccnxTlvDictionaryEntryTypeToString(entry->entryType), (void *) entry->_entry.vec);
    ccnxCodecNetworkBufferIoVec_Display(entry->_entry.vec, 6);
}

static void
_ccnxTlvDictionary_DisplayJson(const _CCNxTlvDictionaryEntry *entry, int index)
{
    printf("     Entry %3d type %8s pointer %p\n", index, _ccnxTlvDictionaryEntryTypeToString(entry->entryType), (void *) entry->_entry.json);
    char *string = parcJSON_ToString(entry->_entry.json);
    printf("%s\n", string);
    parcMemory_Deallocate((void **) &string);
}

static void
_ccnxTlvDictionary_DisplayName(const _CCNxTlvDictionaryEntry *entry, int index)
{
    printf("     Entry %3d type %8s pointer %p\n", index, _ccnxTlvDictionaryEntryTypeToString(entry->entryType), (void *) entry->_entry.name);
    ccnxName_Display(entry->_entry.name, 6);
}

static void
_ccnxTlvDictionary_DisplayUnknown(const _CCNxTlvDictionaryEntry *entry, int index)
{
    printf("     Entry %3d type %8s pointer %p\n", index, _ccnxTlvDictionaryEntryTypeToString(entry->entryType), (void *) entry->_entry.buffer);
}

static void
_ccnxTlvDictionary_DisplayListEntry(const _CCNxTlvDictionaryListEntry *entry, int listIndex, int position)
{
    printf("     List %3d Position %3d key 0x%04X pointer %p\n", listIndex, position, entry->key, (void *) entry->buffer);
    parcBuffer_Display(entry->buffer, 6);
}

void
ccnxTlvDictionary_Display(const CCNxTlvDictionary *dictionary, int indent)
{
    parcDisplayIndented_PrintLine(indent, "CCNxTlvDictionary@%p fastArraySize %zu listSize %zu dictionaryType %s schemaVersion %d refcount %" PRIu64 "\n",
                                  (void *) dictionary,
                                  dictionary->fastArraySize,
                                  dictionary->listSize,
                                  _ccnxTlvDictionaryTypeToString(dictionary->dictionaryType),
                                  dictionary->schemaVersion,
                                  parcObject_GetReferenceCount((PARCObject *) dictionary));

    parcDisplayIndented_PrintLine(indent, "    createTime %0.6f generation %u Info %p InfoFreeFunc %p\n",
                                  (dictionary->creationTime.tv_sec + dictionary->creationTime.tv_usec * 1E-6),
                                  dictionary->generation,
                                  (void *) dictionary->info,
                                  dictionary->infoFreeFunction);

    for (int i = 0; i < dictionary->fastArraySize; i++) {
        if (dictionary->directArray[i].entryType != ENTRY_UNSET) {
            switch (dictionary->directArray[i].entryType) {
                case ENTRY_BUFFER:
                    _ccnxTlvDictionary_DisplayBuffer(&dictionary->directArray[i], i);
                    break;

                case ENTRY_INTEGER:
                    _ccnxTlvDictionary_DisplayInteger(&dictionary->directArray[i], i);
                    break;

                case ENTRY_IOVEC:
                    _ccnxTlvDictionary_DisplayIoVec(&dictionary->directArray[i], i);
                    break;

                case ENTRY_JSON:
                    _ccnxTlvDictionary_DisplayJson(&dictionary->directArray[i], i);
                    break;

                case ENTRY_NAME:
                    _ccnxTlvDictionary_DisplayName(&dictionary->directArray[i], i);
                    break;

                default:
                    _ccnxTlvDictionary_DisplayUnknown(&dictionary->directArray[i], i);
            }
        }
    }

    for (int i = 0; i < dictionary->listSize; i++) {
        _CCNxTlvDictionaryListEntry *entry = _getListHead(dictionary, i);
        if (entry) {
            int position = 0;
            printf("   Displaying custom entry list index %3d head %p\n", i, (void *) entry);
            while (entry) {
                _ccnxTlvDictionary_DisplayListEntry(entry, i, position);
                entry = entry->next;
            }
        }
    }
}

static bool
_ccnxTlvDictionaryEntry_Equals(const _CCNxTlvDictionaryEntry *a, const _CCNxTlvDictionaryEntry *b)
{
    if (a == NULL && b == NULL) {
        return true;
    }

    if (a == NULL || b == NULL) {
        return false;
    }

    bool equals = false;
    if (a->entryType == b->entryType) {
        switch (a->entryType) {
            case ENTRY_UNSET:
                equals = true;
                break;

            case ENTRY_BUFFER:
                equals = parcBuffer_Equals(a->_entry.buffer, b->_entry.buffer);
                break;

            case ENTRY_OBJECT:
                equals = parcObject_Equals(a->_entry.object, b->_entry.object);
                break;

            case ENTRY_INTEGER:
                equals = (a->_entry.integer == b->_entry.integer);
                break;

            case ENTRY_IOVEC:
                equals = ccnxCodecNetworkBufferIoVec_Equals(a->_entry.vec, b->_entry.vec);
                break;

            case ENTRY_JSON:
                equals = parcJSON_Equals(a->_entry.json, b->_entry.json);
                break;

            case ENTRY_NAME:
                equals = ccnxName_Equals(a->_entry.name, b->_entry.name);
                break;

            default:
                trapIllegalValue(a->entryType, "Cannot compare due to unknown entry type: %d", a->entryType);
        }
    }
    return equals;
}

static bool
_ccnxTlvDictionaryListEntry_Equals(const _CCNxTlvDictionaryListEntry *a, const _CCNxTlvDictionaryListEntry *b)
{
    if (a == NULL && b == NULL) {
        return true;
    }

    if (a == NULL || b == NULL) {
        return false;
    }

    if (a->key == b->key) {
        if (parcBuffer_Equals(a->buffer, b->buffer)) {
            return true;
        }
    }
    return false;
}

static bool
_ccnxTlvDictionary_ListEquals(const _CCNxTlvDictionaryListEntry *listHeadA, const _CCNxTlvDictionaryListEntry *listHeadB)
{
    if (listHeadA == NULL && listHeadB == NULL) {
        return true;
    }

    if (listHeadA == NULL || listHeadB == NULL) {
        return false;
    }

    // walk both linked lists in parallel
    while (listHeadA && listHeadB) {
        if (!_ccnxTlvDictionaryListEntry_Equals(listHeadA, listHeadB)) {
            return false;
        }

        listHeadA = listHeadA->next;
        listHeadB = listHeadB->next;
    }

    // they must both be NULL otherwise the lists did not end at same place
    if (listHeadA == NULL && listHeadB == NULL) {
        return true;
    }
    return false;
}

/*
 * precondition: we know they are not null and they have the same fastarray size
 */
static bool
_ccnxTlvDictionary_FastArrayEquals(const CCNxTlvDictionary *a, const CCNxTlvDictionary *b)
{
    bool equals = true;
    for (int i = 0; i < a->fastArraySize && equals; i++) {
        equals = _ccnxTlvDictionaryEntry_Equals(&a->directArray[i], &b->directArray[i]);
    }
    return equals;
}

/*
 * preconditiona: we know they are not null and they have the same list size
 */
static bool
_ccnxTlvDictionary_ListsEquals(const CCNxTlvDictionary *a, const CCNxTlvDictionary *b)
{
    bool equals = true;
    for (int i = 0; i < a->listSize && equals; i++) {
        _CCNxTlvDictionaryListEntry *entry_a = _getListHead(a, i);
        _CCNxTlvDictionaryListEntry *entry_b = _getListHead(b, i);
        equals = _ccnxTlvDictionary_ListEquals(entry_a, entry_b);
    }
    return equals;
}

bool
ccnxTlvDictionary_Equals(const CCNxTlvDictionary *a, const CCNxTlvDictionary *b)
{
    if (a == NULL && b == NULL) {
        return true;
    }

    if (a == NULL || b == NULL) {
        return false;
    }

    // They are both non-null
    bool equals = false;
    if (a->fastArraySize == b->fastArraySize) {
        if (a->listSize == b->listSize) {
            if (a->dictionaryType == b->dictionaryType) {
                if (a->schemaVersion == b->schemaVersion) {
                    if (_ccnxTlvDictionary_FastArrayEquals(a, b)) {
                        if (_ccnxTlvDictionary_ListsEquals(a, b)) {
                            equals = true;
                        }
                    }
                }
            }
        }
    }
    return equals;
}
