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
#include <stdio.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>

#include <LongBow/runtime.h>

#include <ccnx/common/codec/ccnxCodec_EncodingBuffer.h>

struct ccnx_tlv_encoding_buffer_linked_array;
typedef struct ccnx_tlv_encoding_buffer_linked_array _CCNxCodecEncodingBufferLinkedArray;

typedef struct array_entry {
    struct iovec vec;
    PARCBuffer *buffer;
} _ArrayEntry;

/**
 * @typedef CCNxCodecEncodingBufferLinkedArray
 * @abstract A chain of arrays of PARCBuffers
 * @constant next The next EncodingBuffer in the chain
 * @constant prev The previous EncodingBuffer in the chain
 * @constant cacapcity The array capacity of bufferArray
 * @constant count The number of items in this EncodingBuffer
 * @constant bytes The bytes in this EncodingBuffer
 * @constant bufferArray An array of (PARCBuffer *)
 * @discussion If 'next' is NULL, then new elements are appendig to bufferArray.  If 'next' is
 *             not null, then calls to Append will go to the bufferArray at the tail of the linked list.
 */
struct ccnx_tlv_encoding_buffer_linked_array {
    // we can chain these encoding buffers together to make
    // one long linear list
    _CCNxCodecEncodingBufferLinkedArray *next;
    _CCNxCodecEncodingBufferLinkedArray *prev;

    // the number of elements allocated the array
    uint32_t capacity;

    // The total number of elements in this bufferArray
    uint32_t count;

    // The total bytes of this bufferArray
    size_t bytes;

    // each encoding buffer is an array of _ArrayEntry structures containing
    // a PARCBuffer reference and a vector referencing the PARCBuffer contents.
    _ArrayEntry *array;
};

/**
 * @typedef CCNxCodecEncodingBuffer
 * @abstract A chain of arrays of PARCBuffers
 * @constant next The next EncodingBuffer in the chain
 * @constant prev The previous EncodingBuffer in the chain
 * @constant cacapcity The array capacity of bufferArray
 * @constant totalCount The total number of items in this EncodingBuffer and all subsequent
 * @constant totalBytes The total number of bytes in this EncodingBuffer and all subsequent
 * @constant bufferArray An array of (PARCBuffer *)
 * @discussion If 'next' is NULL, then new elements are appendig to bufferArray.  If 'next' is
 *             not null, then calls to Append will go to the bufferArray at the tail of the linked list.
 */
struct ccnx_codec_encoding_buffer {
    _CCNxCodecEncodingBufferLinkedArray *head;
    _CCNxCodecEncodingBufferLinkedArray *tail;

    // The total number of elements in all LinkedArrays
    uint32_t totalCount;

    // The total bytes in all LinkedArrays
    size_t totalBytes;
};

static void _ccnxCodecEncodingBufferEntry_SetIOVec(PARCBuffer *buffer, struct iovec *iov);

static const uint32_t DEFAULT_CAPACITY = 32;

// ======================================================================================
// CCNxCodecEncodingBufferLinkedArray releated

static void
_ccnxCodecEncodingBufferLinkedArray_Display(const _CCNxCodecEncodingBufferLinkedArray *array,
                                            int indentation)
{
    printf("Entry %p prev %p next %p capacity %u count %u bytes %zu\n",
           (void *) array, (void *) array->prev, (void *) array->next, array->capacity, array->count, array->bytes);

    size_t totalBytes = 0;

    for (int i = 0; i < array->count; i++) {
        size_t bytes = array->array[i].vec.iov_len;
        totalBytes += bytes;
        printf("    %3d iovec_base=%p bytes=%4zu total bytes=%4zu\n",
               i, array->array[i].vec.iov_base, bytes, totalBytes);
    }
}

static _CCNxCodecEncodingBufferLinkedArray *
_ccnxCodecEncodingBufferLinkedArray_Create(uint32_t capacity)
{
    // allocation for the object plus the array of buffers
    _CCNxCodecEncodingBufferLinkedArray *array = parcMemory_Allocate(sizeof(_CCNxCodecEncodingBufferLinkedArray));
    assertNotNull(array, "parcMemory_Allocate(%zu) returned NULL", sizeof(_CCNxCodecEncodingBufferLinkedArray));
    array->capacity = capacity;
    array->bytes = 0;
    array->count = 0;
    array->next = NULL;
    array->prev = NULL;
    array->array = parcMemory_AllocateAndClear(sizeof(_ArrayEntry) * capacity);
    return array;
}

/**
 * A LinkedArray can only be released if it has been removed from the EncodingBuffer
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
 */
static void
_ccnxCodecEncodingBufferLinkedArray_Release(_CCNxCodecEncodingBufferLinkedArray **arrayPtr)
{
    assertNotNull(arrayPtr, "Parameter must be non-null");
    _CCNxCodecEncodingBufferLinkedArray *array = *arrayPtr;

    assertNull(array->prev, "array->prev must be null")
    {
        _ccnxCodecEncodingBufferLinkedArray_Display(array, 0);
    }

    assertNull(array->next, "array->next must be null")
    {
        _ccnxCodecEncodingBufferLinkedArray_Display(array, 0);
    }

    for (int i = 0; i < array->count; i++) {
        if (array->array[i].buffer) {
            parcBuffer_Release(&(array->array[i].buffer));
        }
    }

    parcMemory_Deallocate(&(array->array));
    parcMemory_Deallocate((void **) &array);
}

static void
_ccnxCodecEncodingBufferLinkedArray_Validate(const _CCNxCodecEncodingBufferLinkedArray *array)
{
    assertNotNull(array, "Parameter list must be non-null");

    if (array->next) {
        assertTrue(array->next->prev == array,
                   "next->prev does not point to this entry: entry %p next %p next->prev %p",
                   (void *) array, (void *) array->next, (void *) array->next->prev)
        {
            _ccnxCodecEncodingBufferLinkedArray_Display(array, 0);
        }
    }

    if (array->prev) {
        assertTrue(array->prev->next == array,
                   "prev->next does not point to this entry: entry %p prev %p prev->next %p",
                   (void *) array, (void *) array->prev, (void *) array->prev->next)
        {
            _ccnxCodecEncodingBufferLinkedArray_Display(array, 0);
        }
    }

    assertTrue(array->count <= array->capacity,
               "Array count greater than capacity: count %u capacity %u",
               array->count, array->capacity)
    {
        _ccnxCodecEncodingBufferLinkedArray_Display(array, 0);
    }

    size_t totalBytes = 0;

    for (int i = 0; i < array->count; i++) {
        totalBytes += array->array[i].vec.iov_len;
    }

    assertTrue(totalBytes == array->bytes,
               "Array bytes wrong, got %zu expected %zu",
               totalBytes, array->bytes);
}

__attribute__((unused))
static void
_ccnxCodecEncodingBuffer_Validate(CCNxCodecEncodingBuffer *list)
{
    assertNotNull(list, "List is null");

    // either we have both a head and a tail or neither
    assertTrue((list->head && list->tail) || !(list->head || list->tail),
               "List has a mixture of null head or tail: list %p head %p tail %p",
               (void *) list, (void *) list->head, (void *) list->tail)
    {
        ccnxCodecEncodingBuffer_Display(list, 0);
    }

    assertTrue(list->head == NULL || list->head->prev == NULL,
               "List head has head->prev: list %p head %p head->prev %p",
               (void *) list, (void *) list->head, (void *) list->tail)
    {
        ccnxCodecEncodingBuffer_Display(list, 0);
    }

    assertTrue(list->tail == NULL || list->tail->next == NULL,
               "List tail has tail->next: list %p tail %p tail->next %p",
               (void *) list, (void *) list->head, (void *) list->tail)
    {
        ccnxCodecEncodingBuffer_Display(list, 0);
    }


    // walk the linked list and make sure the count is equal to what we expect
    size_t itemCount = 0;
    size_t totalBytes = 0;
    _CCNxCodecEncodingBufferLinkedArray *next = list->head;
    while (next != NULL) {
        _ccnxCodecEncodingBufferLinkedArray_Validate(next);

        itemCount += next->count;
        totalBytes += next->bytes;

        if (next->next == NULL) {
            assertTrue(next == list->tail,
                       "Found list link with null next, but it is not tail: list %p list->tail %p entry %p",
                       (void *) list, (void *) list->tail, (void *) next)
            {
                ccnxCodecEncodingBuffer_Display(list, 0);
                _ccnxCodecEncodingBufferLinkedArray_Display(next, 0);
            }
        }
        next = next->next;
    }

    assertTrue(itemCount == list->totalCount, "Wrong itemCount, got %zu expected %u", itemCount, list->totalCount);
    assertTrue(totalBytes == list->totalBytes, "Wrong totalBytes, got %zu expected %zu", totalBytes, list->totalBytes);
}


// ======================================================================================

static void
_ccnxCodecEncodingBuffer_Remove(CCNxCodecEncodingBuffer *list, _CCNxCodecEncodingBufferLinkedArray *array)
{
    if (array->prev) {
        array->prev->next = array->next;
    }

    if (array->next) {
        array->next->prev = array->prev;
    }

    if (list->head == array) {
        list->head = array->next;
    } else if (list->tail == array) {
        list->tail = array->prev;
    }

    array->next = NULL;
    array->prev = NULL;

    assertTrue(list->totalBytes >= array->bytes,
               "list bytes smaller than array: list %zu array %zu", list->totalBytes, array->bytes);
    assertTrue(list->totalCount >= array->count,
               "list count smaller than array: list %u array %u", list->totalCount, array->count);

    list->totalCount -= array->count;
    list->totalBytes -= array->bytes;
}

static void
_ccnxCodecEncodingBuffer_FinalRelease(CCNxCodecEncodingBuffer **listBufferPtr)
{
    CCNxCodecEncodingBuffer *list = *listBufferPtr;

    _CCNxCodecEncodingBufferLinkedArray *next = list->head;
    while (next != NULL) {
        _CCNxCodecEncodingBufferLinkedArray *nextnext = next->next;
        _ccnxCodecEncodingBuffer_Remove(list, next);
        _ccnxCodecEncodingBufferLinkedArray_Release(&next);
        next = nextnext;
    }
}

parcObject_ExtendPARCObject(CCNxCodecEncodingBuffer, _ccnxCodecEncodingBuffer_FinalRelease, NULL, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementAcquire(ccnxCodecEncodingBuffer, CCNxCodecEncodingBuffer);

parcObject_ImplementRelease(ccnxCodecEncodingBuffer, CCNxCodecEncodingBuffer);

CCNxCodecEncodingBuffer *
ccnxCodecEncodingBuffer_Create(void)
{
    CCNxCodecEncodingBuffer *listBuffer = parcObject_CreateInstance(CCNxCodecEncodingBuffer);
    listBuffer->head = NULL;
    listBuffer->tail = NULL;
    listBuffer->totalCount = 0;
    listBuffer->totalBytes = 0;
    return listBuffer;
}

void
ccnxCodecEncodingBuffer_Display(const CCNxCodecEncodingBuffer *list, int indentation)
{
    printf("List %p head %p tail %p itemCount %u totalBytes %zu\n",
           (void *) list, (void *) list->head, (void *) list->tail, list->totalCount, list->totalBytes);

    size_t totalCount = 0;
    size_t totalBytes = 0;
    size_t position = 0;
    _CCNxCodecEncodingBufferLinkedArray *next = list->head;
    while (next != NULL) {
        printf(" %3zu: entry %p prev %p next %p\n",
               position, (void *) next, (void *) next->prev, (void *) next->next);
        _ccnxCodecEncodingBufferLinkedArray_Display(next, indentation);

        totalCount += next->count;
        totalBytes += next->bytes;
        position++;
        next = next->next;
    }
}

static void
_ccnxCodecEncodingBuffer_AppendLinkedArray(CCNxCodecEncodingBuffer *list, _CCNxCodecEncodingBufferLinkedArray *array)
{
    if (list->tail) {
        list->tail->next = array;
    } else {
        // if list tail is null, it means list head is null too
        list->head = array;
    }

    array->prev = list->tail;
    list->tail = array;
}

static void
_ccnxCodecEncodingBuffer_PrependLinkedArray(CCNxCodecEncodingBuffer *list, _CCNxCodecEncodingBufferLinkedArray *array)
{
    array->next = list->head;
    array->prev = list->head->prev;
    list->head->prev = array;
    list->head = array;
}

size_t
ccnxCodecEncodingBuffer_PrependBuffer(CCNxCodecEncodingBuffer *list, PARCBuffer *buffer)
{
    assertNotNull(list, "Parameter list must be non-null");
    assertNotNull(buffer, "Parameter buffer must be non-null");

    _CCNxCodecEncodingBufferLinkedArray *head = list->head;
    if ((head == NULL) || (list->head->count == list->head->capacity)) {
        head = _ccnxCodecEncodingBufferLinkedArray_Create(DEFAULT_CAPACITY);
        _ccnxCodecEncodingBuffer_PrependLinkedArray(list, head);
    }

    assertTrue(head->count < head->capacity, "head does not have any room left")
    {
        _ccnxCodecEncodingBufferLinkedArray_Display(head, 0);
    }

    size_t position = list->head->count;
    for (int i = 0; i < list->head->count; i++) {
        head->array[i + 1] = head->array[i];
    }
    head->array[0].buffer = parcBuffer_Acquire(buffer);
    _ccnxCodecEncodingBufferEntry_SetIOVec(buffer, &head->array[0].vec);

    size_t bytes = head->array[0].vec.iov_len;
    head->bytes += bytes;
    list->totalBytes += bytes;

    head->count++;
    list->totalCount++;

    return position;
}

size_t
ccnxCodecEncodingBuffer_AppendBuffer(CCNxCodecEncodingBuffer *list, PARCBuffer *buffer)
{
    assertNotNull(list, "Parameter list must be non-null");
    assertNotNull(buffer, "Parameter buffer must be non-null");

    _CCNxCodecEncodingBufferLinkedArray *tail = list->tail;
    if (tail == NULL || list->tail->count == list->tail->capacity) {
        tail = _ccnxCodecEncodingBufferLinkedArray_Create(DEFAULT_CAPACITY);
        _ccnxCodecEncodingBuffer_AppendLinkedArray(list, tail);
    }

    assertTrue(tail->count < tail->capacity, "tail does not have any room left")
    {
        _ccnxCodecEncodingBufferLinkedArray_Display(tail, 0);
    }

    size_t position = list->totalCount;
    tail->array[tail->count].buffer = parcBuffer_Acquire(buffer);
    _ccnxCodecEncodingBufferEntry_SetIOVec(buffer, &tail->array[tail->count].vec);

    size_t bytes = tail->array[tail->count].vec.iov_len;
    tail->bytes += bytes;
    list->totalBytes += bytes;

    tail->count++;
    list->totalCount++;

    return position;
}

// Returns the number of elements in the list
size_t
ccnxCodecEncodingBuffer_Size(const CCNxCodecEncodingBuffer *buffer)
{
    assertNotNull(buffer, "Parameter buffer must be non-null");
    return buffer->totalCount;
}

// Returns the total number of bytes in the list
size_t
ccnxCodecEncodingBuffer_Length(const CCNxCodecEncodingBuffer *buffer)
{
    assertNotNull(buffer, "Parameter buffer must be non-null");
    return buffer->totalBytes;
}

// ======================================================================================

// ======================================================================================

static void
_ccnxCodecEncodingBufferEntry_SetIOVec(PARCBuffer *buffer, struct iovec *iov)
{
    // this can return NULL for a 0 capacity entry
    PARCByteArray *byteArray = parcBuffer_Array(buffer);

    if (byteArray) {
        uint8_t *array = parcByteArray_Array(byteArray);

        // we need to advance the array so it is at the buffer's offset.
        size_t offset = parcBuffer_ArrayOffset(buffer) + parcBuffer_Position(buffer);

        iov->iov_base = (array + offset);
        iov->iov_len = parcBuffer_Remaining(buffer);
    } else {
        iov->iov_base = NULL;
        iov->iov_len = 0;
    }
}

// Creates an iovec array pointing to the PARCBuffer contents at offset for length
CCNxCodecEncodingBuffer *
ccnxCodecEncodingBuffer_Slice(CCNxCodecEncodingBuffer *encodingBuffer, size_t offset, size_t length)
{
    CCNxCodecEncodingBuffer *listBuffer = parcObject_CreateInstance(CCNxCodecEncodingBuffer);
    listBuffer->head = NULL;
    listBuffer->tail = NULL;
    listBuffer->totalCount = 0;
    listBuffer->totalBytes = 0;

    _CCNxCodecEncodingBufferLinkedArray *head;
    head = _ccnxCodecEncodingBufferLinkedArray_Create(encodingBuffer->totalCount); // pessimistic
    _ccnxCodecEncodingBuffer_AppendLinkedArray(listBuffer, head);

    _CCNxCodecEncodingBufferLinkedArray *next = encodingBuffer->head;
    int position = 0;
    while (next && length) {
        for (int i = 0; (i < next->count) && length; i++) {
            if ((offset >= position) && (offset < (position + next->array[i].vec.iov_len))) {
                int remainder = 0;
                head->array[head->count].buffer = parcBuffer_Acquire(next->array[i].buffer);
                head->array[head->count].vec.iov_base = next->array[i].vec.iov_base + (offset - position);
                remainder = next->array[i].vec.iov_len - (offset - position);
                if (remainder > length) {
                    remainder = length;
                }
                head->array[head->count].vec.iov_len = remainder;
                offset += remainder;
                length -= remainder;

                head->count++;
                head->bytes += remainder;
                listBuffer->totalCount++;
                listBuffer->totalBytes += remainder;
            }
            position += next->array[i].vec.iov_len;
        }
        next = next->next;
    }
    if (listBuffer->totalCount == 0) {
        ccnxCodecEncodingBuffer_Release(&listBuffer);
    }

    return listBuffer;
}

// Creates an iovec array pointing to the PARCBuffer contents
CCNxCodecEncodingBufferIOVec *
ccnxCodecEncodingBuffer_CreateIOVec(CCNxCodecEncodingBuffer *encodingBuffer)
{
    // one allocation for the object and the array
    size_t totalAllocation = sizeof(CCNxCodecEncodingBufferIOVec) + sizeof(struct iovec) * encodingBuffer->totalCount;

    CCNxCodecEncodingBufferIOVec *vec = parcMemory_Allocate(totalAllocation);
    assertNotNull(vec, "parcMemory_Allocate(%zu) returned NULL", totalAllocation);
    vec->encodingBuffer = ccnxCodecEncodingBuffer_Acquire(encodingBuffer);
    vec->iovcnt = (int) encodingBuffer->totalCount;

    _CCNxCodecEncodingBufferLinkedArray *next = encodingBuffer->head;
    int position = 0;
    while (next) {
        for (int i = 0; i < next->count; i++) {
            vec->iov[position] = next->array[i].vec;
            position++;
        }
        next = next->next;
    }

    return vec;
}

// releases the iovec structure, but does not release the contents.  you must
// keep at least one reference to the parent CCNxCodecEncodingBuffer alive.
void
ccnxCodecEncodingBufferIOVec_Release(CCNxCodecEncodingBufferIOVec **iovecPtr)
{
    CCNxCodecEncodingBufferIOVec *iov = *iovecPtr;
    ccnxCodecEncodingBuffer_Release(&iov->encodingBuffer);
    parcMemory_Deallocate((void **) &iov);
    *iovecPtr = NULL;
}
