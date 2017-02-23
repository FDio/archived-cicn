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
 * We maintain a linked list of memory blocks.  We fill each memory block to capacity, then
 * allocate another memory block, putting it at the tail of the list.
 *
 * We maintain a "current" pointer to the memory block that holds "position".  Insertions always go
 * current block.
 *
 * Each memory block has a capacity and a limit.  The capacity is the maximum number of bytes available.
 * The limit is the furthest byte written.  It will not exceed the capacity..
 *
 * Once a memory block has a "next" block, the limit is fixed.  One cannot shrink or expand the limit.
 * When the "next" pointer is set, the capacity is shrunk to the limit and the buffer is called "frozen".
 *
 *    (always in ABSOLUTE bytes)
 *                                                                         position = 4036
 *    begin = 0                  begin = 1536               begin = 3536   |
 *    |                          |                          |              |
 *   +--------------------------+--------------------------+--------------------------+
 *   |         block 0          |         block 1          |         block 2          |
 *   +--------------------------+--------------------------+--------------------------+
 *                             |                       |                  |           |
 *                          capacity = 1536        capacity = 2000        |       capacity = 2046
 *                          limit = 1536           limit = 2000       limit = 500
 *    (always in RELATIVE bytes)
 *
 *  Block 0 was allocated at 1536 bytes and filled to capacity before it was frozen.
 *
 *  Block 1 was allocated at 2046 but only filled to 2000 bytes when it was frozen.  The last 46 bytes
 *          of the block are permanently lost.
 *
 *  Block 2 is still in use.  500 bytes have been written out of the 2046 capacity.
 *
 * The "begin" of a memory block is equal to the previous's memory block's "begin" plus
 * the previous blocks "limit" when it is frozen.  The "begin" value is absolute byte position
 * and it will never change because all prior blocks must be frozen.
 *
 * The total "limit" of the entire chain is the tail's "begin" plus tail's "limit".
 *
 *
 */

#include <config.h>
#include <stdio.h>
#include <parc/algol/parc_Memory.h>
#include <LongBow/runtime.h>

#include <ccnx/common/codec/ccnxCodec_NetworkBuffer.h>

struct ccnx_codec_network_buffer_memory;
typedef struct ccnx_codec_network_buffer_memory CCNxCodecNetworkBufferMemory;

struct ccnx_codec_network_buffer_memory {
    CCNxCodecNetworkBufferMemory *next;

    size_t begin;      /**< Absolute position of begining */
    size_t limit;      /**< Bytes used */
    size_t capacity;   /**< maximum bytes available (end - begin) */

    uint8_t *memory;
};

struct ccnx_codec_network_buffer_iovec {
    CCNxCodecNetworkBuffer *networkBuffer;
    unsigned refcount;
    size_t totalBytes;
    int iovcnt;
    struct iovec array[];
};

struct ccnx_codec_network_buffer {
    size_t position;
    size_t capacity;         /**< Bytes allocated */

    CCNxCodecNetworkBufferMemory *current;
    CCNxCodecNetworkBufferMemory *head;
    CCNxCodecNetworkBufferMemory *tail;

    void *userarg;
    CCNxCodecNetworkBufferMemoryBlockFunctions memoryFunctions;
    unsigned refcount;
};

// ================================================================================

#define INLINE_POSITION(block) ((uint8_t *) block + sizeof(CCNxCodecNetworkBufferMemory))

static CCNxCodecNetworkBufferMemory *
_ccnxCodecNetworkBufferMemory_Allocate(CCNxCodecNetworkBuffer *buffer, size_t bytes)
{
    assertNotNull(buffer->memoryFunctions.allocator, "Allocator must be non-null to allocate memory!");

    CCNxCodecNetworkBufferMemory *block;
    size_t totalAllocation = bytes + sizeof(CCNxCodecNetworkBufferMemory);
    size_t actual = buffer->memoryFunctions.allocator(buffer->userarg, totalAllocation, (void **) &block);

    if (actual > sizeof(CCNxCodecNetworkBufferMemory)) {
        block->next = NULL;
        block->begin = 0;
        block->capacity = actual - sizeof(CCNxCodecNetworkBufferMemory);
        block->limit = 0;

        block->memory = INLINE_POSITION(block);
        return block;
    }

    // Need a de-allocator, see case 1006
    trapOutOfMemory("Wanted %zu got %zu, minimum required %zu", totalAllocation, actual, sizeof(CCNxCodecNetworkBufferMemory));
    return NULL;
}

/**
 * Wrap a user-provided buffer.  It will be de-allocated with the buffer memory functions.
 *
 * The capacity = limit = length of the user provided memory.
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
static CCNxCodecNetworkBufferMemory *
_ccnxCodecNetworkBufferMemory_Wrap(CCNxCodecNetworkBuffer *buffer, size_t length, uint8_t memory[length])
{
    CCNxCodecNetworkBufferMemory *block = parcMemory_AllocateAndClear(sizeof(CCNxCodecNetworkBufferMemory));
    if (block) {
        block->next = NULL;
        block->begin = 0;
        block->capacity = length;
        block->limit = length;
        block->memory = memory;

        return block;
    }
    trapOutOfMemory("Could not allocate a CCNxCodecNetworkBufferMemory");
}

/**
 * Releases a memory block
 *
 * The memory block must not be in a linked list (i.e memory->next must be NULL)
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
_ccnxCodecNetworkBufferMemory_Release(CCNxCodecNetworkBuffer *buffer, CCNxCodecNetworkBufferMemory **memoryPtr)
{
    assertNotNull(memoryPtr, "Parameter must be non-null");
    assertNotNull(*memoryPtr, "Parameter must dereference to non-null");

    CCNxCodecNetworkBufferMemory *memory = *memoryPtr;

    assertNull(memory->next, "memory->next is not null");

    // If the memory is not in-line, free it with the deallocator
    if (memory->memory == INLINE_POSITION(memory)) {
        if (buffer->memoryFunctions.deallocator) {
            buffer->memoryFunctions.deallocator(buffer->userarg, (void **) memoryPtr);
        }
    } else {
        if (buffer->memoryFunctions.deallocator) {
            buffer->memoryFunctions.deallocator(buffer->userarg, (void **) &memory->memory);
        }
        parcMemory_Deallocate((void **) &memory);
    }


    *memoryPtr = NULL;
}

static void
_ccnxCodecNetworkBufferMemory_Display(const CCNxCodecNetworkBufferMemory *block, unsigned indent)
{
    assertNotNull(block, "Parameter block must be non-null");

    printf("Memory block %p next %p offset %zu limit %zu capacity %zu\n",
           (void *) block, (void *) block->next, block->begin, block->limit, block->capacity);

    longBowDebug_MemoryDump((const char *) block->memory, block->capacity);
}

static bool
_ccnxCodecNetworkBufferMemory_ContainsPosition(CCNxCodecNetworkBufferMemory *memory, size_t position)
{
    return (memory->begin <= position && position < memory->begin + memory->limit);
}

// ================================================================================

static void
_ccnxCodecNetworkBuffer_Expand(CCNxCodecNetworkBuffer *buffer)
{
    size_t allocationSize = 2048;
    CCNxCodecNetworkBufferMemory *memory = _ccnxCodecNetworkBufferMemory_Allocate(buffer, allocationSize);

    buffer->capacity += memory->capacity;

    memory->begin = buffer->tail->begin + buffer->tail->limit;

    // this will free the tail buffer.  We will drop its capacity to its limit.
    buffer->tail->next = memory;
    buffer->tail->capacity = buffer->tail->limit;

    buffer->tail = memory;
}

static size_t
_ccnxCodecNetworkBuffer_RemainingCurrentBlock(CCNxCodecNetworkBuffer *buffer)
{
    size_t remaining = buffer->current->begin + buffer->current->capacity - buffer->position;
    return remaining;
}

static size_t
_ccnxCodecNetworkBuffer_BlockCount(CCNxCodecNetworkBuffer *buffer)
{
    // we should store this count for faster access
    size_t count = 0;
    CCNxCodecNetworkBufferMemory *block = buffer->head;
    while (block) {
        count++;
        block = block->next;
    }
    return count;
}

static void
_ccnxCodecNetworkBuffer_AllocateIfNeeded(CCNxCodecNetworkBuffer *buffer)
{
    if (buffer->position == buffer->current->begin + buffer->current->capacity) {
        if (buffer->current->next) {
            buffer->current = buffer->current->next;
        } else {
            // we are at the end of the current buffer and there's nothing beyond,
            // so allocate another memory block
            _ccnxCodecNetworkBuffer_Expand(buffer);
            buffer->current = buffer->tail;
        }
    }
}

/**
 * Check if we can fit 'length' bytes in contiguous memory.
 *
 * If we cannot, and the remaining buffer space in the current buffer is small, freeze it out
 * and allocate a new buffer.  Otherwise if the difference is large, do not freeze it and the
 * write will span memory blocks.
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
_ccnxCodecNetworkBuffer_EnsureRemaining(CCNxCodecNetworkBuffer *buffer, size_t length)
{
    // If the current block as a next pointer, then the remaining is from
    // the position to the limit.  Otherwise it is from the position to
    // the end.

    size_t remaining;
    remaining = buffer->current->begin + buffer->current->capacity - buffer->position;


    if (remaining < length) {
        // If its a small amount of memory to waste, we'll freeze the curent buffer and
        // make a new one.
        if (length < 32 && buffer->current->next == NULL) {
            _ccnxCodecNetworkBuffer_Expand(buffer);
            buffer->current = buffer->tail;
            return;
        }

        // otherwise, thre is still space in the current buffer, even though it is not
        // long enough.  The writer will just need to span the two memory blocks.
        _ccnxCodecNetworkBuffer_AllocateIfNeeded(buffer);
    }
}


// ================================================================================

static size_t
_ccnxCodecNetworkBuffer_ParcMemoryAllocator(void *userarg, size_t bytes, void **output)
{
    *output = parcMemory_Allocate(bytes);
    if (*output) {
        return bytes;
    }
    return 0;
}

static void
_ccnxCodecNetworkBuffer_ParcMemoryDeallocator(void *userarg, void **memoryPtr)
{
    void *memory = *memoryPtr;
    parcMemory_Deallocate((void **) &memory);
    *memoryPtr = NULL;
}

const CCNxCodecNetworkBufferMemoryBlockFunctions ParcMemoryMemoryBlock = {
    .allocator   = &_ccnxCodecNetworkBuffer_ParcMemoryAllocator,
    .deallocator = &_ccnxCodecNetworkBuffer_ParcMemoryDeallocator
};

CCNxCodecNetworkBuffer *
ccnxCodecNetworkBuffer_Allocate(const CCNxCodecNetworkBufferMemoryBlockFunctions *memoryFunctions, void *userarg)
{
    CCNxCodecNetworkBuffer *buffer = parcMemory_Allocate(sizeof(CCNxCodecNetworkBuffer));
    assertNotNull(buffer, "parcMemory_Allocate(%zu) returned NULL", sizeof(CCNxCodecNetworkBuffer));
    buffer->refcount = 1;
    buffer->position = 0;
    memcpy(&buffer->memoryFunctions, memoryFunctions, sizeof(CCNxCodecNetworkBufferMemoryBlockFunctions));
    buffer->userarg = userarg;
    return buffer;
}

CCNxCodecNetworkBuffer *
ccnxCodecNetworkBuffer_Create(const CCNxCodecNetworkBufferMemoryBlockFunctions *memoryFunctions, void *userarg)
{
    CCNxCodecNetworkBuffer *buffer = ccnxCodecNetworkBuffer_Allocate(memoryFunctions, userarg);

    buffer->head = _ccnxCodecNetworkBufferMemory_Allocate(buffer, 1536);
    buffer->tail = buffer->head;
    buffer->current = buffer->head;
    buffer->capacity = buffer->head->capacity;

    return buffer;
}

CCNxCodecNetworkBuffer *
ccnxCodecNetworkBuffer_CreateFromArray(const CCNxCodecNetworkBufferMemoryBlockFunctions *memoryFunctions, void *userarg, size_t length, uint8_t memory[length])
{
    CCNxCodecNetworkBuffer *buffer = ccnxCodecNetworkBuffer_Allocate(memoryFunctions, userarg);

    buffer->head = _ccnxCodecNetworkBufferMemory_Wrap(buffer, length, memory);
    buffer->tail = buffer->head;
    buffer->current = buffer->head;
    buffer->capacity = buffer->head->capacity;

    return buffer;
}



CCNxCodecNetworkBuffer *
ccnxCodecNetworkBuffer_Acquire(CCNxCodecNetworkBuffer *original)
{
    assertNotNull(original, "Parameter must be non-null");
    assertTrue(original->refcount > 0, "Refcount must be positive, got 0");

    original->refcount++;
    return original;
}

void
ccnxCodecNetworkBuffer_Release(CCNxCodecNetworkBuffer **bufferPtr)
{
    assertNotNull(bufferPtr, "Parameter must be non-null");
    assertNotNull(*bufferPtr, "Parameter must dereference to non-null");

    CCNxCodecNetworkBuffer *buffer = *bufferPtr;
    assertTrue(buffer->refcount > 0, "refcount must be positive");

    buffer->refcount--;
    if (buffer->refcount == 0) {
        while (buffer->head) {
            CCNxCodecNetworkBufferMemory *next = buffer->head->next;
            buffer->head->next = NULL;
            _ccnxCodecNetworkBufferMemory_Release(buffer, &buffer->head);
            buffer->head = next;
        }
        parcMemory_Deallocate((void **) &buffer);
    }
    *bufferPtr = NULL;
}

// ================================================================================

static inline size_t
_ccnxCodecNetworkBuffer_Limit(const CCNxCodecNetworkBuffer *buffer)
{
    return buffer->tail->begin + buffer->tail->limit;
}

size_t
ccnxCodecNetworkBuffer_Position(const CCNxCodecNetworkBuffer *buffer)
{
    assertNotNull(buffer, "Parameter must be non-null");
    return buffer->position;
}

size_t
ccnxCodecNetworkBuffer_Limit(const CCNxCodecNetworkBuffer *buffer)
{
    assertNotNull(buffer, "Parameter must be non-null");
    return _ccnxCodecNetworkBuffer_Limit(buffer);
}

void
ccnxCodecNetworkBuffer_SetPosition(CCNxCodecNetworkBuffer *buffer, size_t position)
{
    assertNotNull(buffer, "Parameter buffer must be non-null");
    assertTrue(position <= _ccnxCodecNetworkBuffer_Limit(buffer), "Position must not exceed limit, got %zu limit %zu",
               position, _ccnxCodecNetworkBuffer_Limit(buffer));

    // We allow the position to be set to the end (just past the last written byte) of the buffer.
    // This is the "next"  position to be written
    if (position == _ccnxCodecNetworkBuffer_Limit(buffer)) {
        buffer->current = buffer->tail;
    } else {
        // Is the new position within the current memory block?
        if (_ccnxCodecNetworkBufferMemory_ContainsPosition(buffer->current, position)) {
            // we're ok, new position is in this buffer, we're done :)
        } else {
            // we need to find the right buffer
            CCNxCodecNetworkBufferMemory *memory = buffer->head;
            while (!_ccnxCodecNetworkBufferMemory_ContainsPosition(memory, position)) {
                memory = memory->next;
                assertNotNull(memory, "Illegal state: position < buffer->limit, but we ran off end of linked list");
            }

            buffer->current = memory;
        }
    }

    buffer->position = position;
}

void
ccnxCodecNetworkBuffer_Finalize(CCNxCodecNetworkBuffer *buffer)
{
    assertNotNull(buffer, "Parameter buffer must be non-null");

    // if we're at the limit, we're done
    if (buffer->position < _ccnxCodecNetworkBuffer_Limit(buffer)) {
        // begin at the tail and free memory blocks until we've found the current position
        size_t position = buffer->position;

        // Is the new position within the current memory block?
        if (_ccnxCodecNetworkBufferMemory_ContainsPosition(buffer->current, position)) {
            // we're ok, new position is in this buffer, we're done :)
        } else {
            // we need to find the right buffer
            CCNxCodecNetworkBufferMemory *memory = buffer->head;
            while (!_ccnxCodecNetworkBufferMemory_ContainsPosition(memory, position)) {
                memory = memory->next;
                assertNotNull(memory, "Illegal state: position < buffer->limit, but we ran off end of linked list");
            }

            buffer->current = memory;
        }

        // discard any memory blocks after this

        CCNxCodecNetworkBufferMemory *current = buffer->current->next;
        while (current) {
            CCNxCodecNetworkBufferMemory *next = current->next;

            // this is a requirement for _Release to not throw an assertion
            current->next = NULL;
            _ccnxCodecNetworkBufferMemory_Release(buffer, &current);
            current = next;
        }

        // Set the limit of the current block so buffer->position is the end
        buffer->current->next = NULL;
        size_t relativePosition = buffer->position - buffer->current->begin;
        buffer->current->limit = relativePosition;
        buffer->tail = buffer->current;
    }
}

static inline void
_ccnxCodecNetworkBuffer_PutUint8(CCNxCodecNetworkBuffer *buffer, uint8_t value)
{
    _ccnxCodecNetworkBuffer_AllocateIfNeeded(buffer);

    size_t relativePosition = buffer->position - buffer->current->begin;
    buffer->current->memory[relativePosition++] = value;
    if (relativePosition > buffer->current->limit) {
        buffer->current->limit = relativePosition;
    }

    buffer->position++;
}

void
ccnxCodecNetworkBuffer_PutUint8(CCNxCodecNetworkBuffer *buffer, uint8_t value)
{
    assertNotNull(buffer, "Parameter buffer must be non-null");
    _ccnxCodecNetworkBuffer_PutUint8(buffer, value);
}

void
ccnxCodecNetworkBuffer_PutUint16(CCNxCodecNetworkBuffer *buffer, uint16_t value)
{
    _ccnxCodecNetworkBuffer_EnsureRemaining(buffer, 2);

    _ccnxCodecNetworkBuffer_PutUint8(buffer, value >> 8);
    _ccnxCodecNetworkBuffer_PutUint8(buffer, value & 0xFF);
}

void
ccnxCodecNetworkBuffer_PutUint32(CCNxCodecNetworkBuffer *buffer, uint32_t value)
{
    _ccnxCodecNetworkBuffer_EnsureRemaining(buffer, 4);

    for (int i = sizeof(uint32_t) - 1; i > 0; i--) {
        uint8_t b = value >> (i * 8) & 0xFF;
        _ccnxCodecNetworkBuffer_PutUint8(buffer, b);
    }

    _ccnxCodecNetworkBuffer_PutUint8(buffer, value & 0xFF);
}

void
ccnxCodecNetworkBuffer_PutUint64(CCNxCodecNetworkBuffer *buffer, uint64_t value)
{
    _ccnxCodecNetworkBuffer_EnsureRemaining(buffer, 8);

    for (int i = sizeof(uint64_t) - 1; i > 0; i--) {
        uint8_t b = value >> (i * 8) & 0xFF;
        _ccnxCodecNetworkBuffer_PutUint8(buffer, b);
    }

    _ccnxCodecNetworkBuffer_PutUint8(buffer, value & 0xFF);
}

void
ccnxCodecNetworkBuffer_PutArray(CCNxCodecNetworkBuffer *buffer, size_t length, const uint8_t array[length])
{
    _ccnxCodecNetworkBuffer_AllocateIfNeeded(buffer);

    size_t offset = 0;
    while (offset < length) {
        size_t available = _ccnxCodecNetworkBuffer_RemainingCurrentBlock(buffer);
        if (available == 0) {
            _ccnxCodecNetworkBuffer_AllocateIfNeeded(buffer);
        } else {
            if (available > (length - offset)) {
                available = length - offset;
            }

            size_t relativePosition = buffer->position - buffer->current->begin;
            void *dest = &buffer->current->memory[relativePosition];
            const void *src = &array[offset];
            memcpy(dest, src, available);

            relativePosition += available;
            if (relativePosition > buffer->current->limit) {
                buffer->current->limit = relativePosition;
            }

            buffer->position += available;
            offset += available;
        }
    }
}

void
ccnxCodecNetworkBuffer_PutBuffer(CCNxCodecNetworkBuffer *buffer, PARCBuffer *value)
{
    size_t length = parcBuffer_Remaining(value);
    if (length > 0) {
        void *ptr = parcBuffer_Overlay(value, 0);
        ccnxCodecNetworkBuffer_PutArray(buffer, length, ptr);
    }
}

PARCBuffer *
ccnxCodecNetworkBuffer_CreateParcBuffer(CCNxCodecNetworkBuffer *buffer)
{
    // We don't have the idea of Flip here yet, so we go from 0 .. position

    size_t length = _ccnxCodecNetworkBuffer_Limit(buffer);
    PARCBuffer *output = parcBuffer_Allocate(length);
    CCNxCodecNetworkBufferMemory *block = buffer->head;
    while (block) {
        size_t available = (length > block->limit) ? block->limit : length;
        if (available > 0) {
            parcBuffer_PutArray(output, available, block->memory);
        }
        length -= available;
        block = block->next;
    }
    parcBuffer_Flip(output);
    return output;
}

PARCSignature *
ccnxCodecNetworkBuffer_ComputeSignature(CCNxCodecNetworkBuffer *buffer, size_t start, size_t end, PARCSigner *signer)
{
    // Most positions (start, end, position, roof) below are in **absolute** coordinates
    // The position relativePosition is relative to the memory block start
    assertNotNull(buffer, "Parameter buffer must be non-null");
    assertTrue(end >= start, "End is less than start: start %zu end %zu", start, end);

    PARCSignature *signature = NULL;
    if (signer) {
        // compute the signature over the specified area

        PARCCryptoHasher *hasher = parcSigner_GetCryptoHasher(signer);
        parcCryptoHasher_Init(hasher);

        size_t position = start;
        CCNxCodecNetworkBufferMemory *block = buffer->head;
        while (block && position < block->begin + block->limit) {
            if (_ccnxCodecNetworkBufferMemory_ContainsPosition(block, position)) {
                // determine if we're going all the way to the block's end or are we
                // stopping early because that's the end of the designated area
                size_t roof = (end > block->begin + block->limit) ? block->limit : end;
                size_t length = roof - position;

                // now calculate the relative offset in the block so we can update the hash
                size_t relativePosition = position - block->begin;

                parcCryptoHasher_UpdateBytes(hasher, &block->memory[relativePosition], length);

                position += length;
            }

            block = block->next;
        }

        PARCCryptoHash *hash = parcCryptoHasher_Finalize(hasher);

        signature = parcSigner_SignDigest(signer, hash);
        parcCryptoHash_Release(&hash);
    }

    return signature;
}

uint8_t
ccnxCodecNetworkBuffer_GetUint8(const CCNxCodecNetworkBuffer *netbuff, size_t position)
{
    assertNotNull(netbuff, "Parameter buffer must be non-null");
    assertTrue(position < _ccnxCodecNetworkBuffer_Limit(netbuff), "Position %zu beyond limit %zu", position, _ccnxCodecNetworkBuffer_Limit(netbuff));

    CCNxCodecNetworkBufferMemory *block = netbuff->head;
    while (block && !_ccnxCodecNetworkBufferMemory_ContainsPosition(block, position)) {
        block = block->next;
    }

    trapUnexpectedStateIf(block == NULL,
                          "Could not find position %zu that is less than limit %zu",
                          position, _ccnxCodecNetworkBuffer_Limit(netbuff));

    size_t relativeOffset = position - block->begin;
    return block->memory[relativeOffset];
}

void
ccnxCodecNetworkBuffer_Display(const CCNxCodecNetworkBuffer *netbuff, unsigned indent)
{
    printf("CCNxCodecNetworkBuffer %p head %p current %p tail %p\n",
           (void *) netbuff, (void *) netbuff->head, (void *) netbuff->current, (void *) netbuff->tail);
    printf(" position %zu limit %zu capacity %zu refcount %u userarg %p\n",
           netbuff->position, _ccnxCodecNetworkBuffer_Limit(netbuff), netbuff->capacity, netbuff->refcount, netbuff->userarg);

    CCNxCodecNetworkBufferMemory *block = netbuff->head;
    while (block) {
        _ccnxCodecNetworkBufferMemory_Display(block, 6);
        block = block->next;
    }
}

// ======================================================================================


CCNxCodecNetworkBufferIoVec *
ccnxCodecNetworkBuffer_CreateIoVec(CCNxCodecNetworkBuffer *buffer)
{
    size_t blockCount = _ccnxCodecNetworkBuffer_BlockCount(buffer);
    size_t allocationSize = sizeof(CCNxCodecNetworkBufferIoVec) + sizeof(struct iovec) * blockCount;

    CCNxCodecNetworkBufferIoVec *vec = parcMemory_Allocate(allocationSize);
    assertNotNull(vec, "parcMemory_Allocate(%zu) returned NULL", allocationSize);
    vec->refcount = 1;
    vec->networkBuffer = ccnxCodecNetworkBuffer_Acquire(buffer);
    vec->iovcnt = (int) blockCount;
    vec->totalBytes = 0;

    CCNxCodecNetworkBufferMemory *block = buffer->head;
    for (int i = 0; i < vec->iovcnt; i++) {
        vec->array[i].iov_base = block->memory;
        vec->array[i].iov_len = block->limit;
        vec->totalBytes += block->limit;
        block = block->next;
    }

    return vec;
}

CCNxCodecNetworkBufferIoVec *
ccnxCodecNetworkBufferIoVec_Acquire(CCNxCodecNetworkBufferIoVec *vec)
{
    assertNotNull(vec, "Parameter vec must be non-null");
    assertTrue(vec->refcount > 0, "Existing reference count is 0");
    vec->refcount++;
    return vec;
}

void
ccnxCodecNetworkBufferIoVec_Release(CCNxCodecNetworkBufferIoVec **vecPtr)
{
    assertNotNull(vecPtr, "Parameter must be non-null");
    assertNotNull(*vecPtr, "Parameter must dereference to non-null");
    CCNxCodecNetworkBufferIoVec *vec = *vecPtr;
    assertTrue(vec->refcount > 0, "object has 0 refcount!");

    vec->refcount--;
    if (vec->refcount == 0) {
        ccnxCodecNetworkBuffer_Release(&vec->networkBuffer);
        parcMemory_Deallocate((void **) &vec);
    }
    *vecPtr = NULL;
}

int
ccnxCodecNetworkBufferIoVec_GetCount(CCNxCodecNetworkBufferIoVec *vec)
{
    assertNotNull(vec, "Parameter vec must be non-null");
    return vec->iovcnt;
}

const struct iovec *
ccnxCodecNetworkBufferIoVec_GetArray(CCNxCodecNetworkBufferIoVec *vec)
{
    assertNotNull(vec, "Parameter vec must be non-null");
    return vec->array;
}

void
ccnxCodecNetworkBufferIoVec_Display(const CCNxCodecNetworkBufferIoVec *vec, int indent)
{
    printf("\nCCNxCodecNetworkBufferIoVec %p refcount %u totalBytes %zu iovcnt %d NetworkBuffer %p\n",
           (void *) vec, vec->refcount, vec->totalBytes, vec->iovcnt, (void *) vec->networkBuffer);

    size_t total = 0;
    for (int i = 0; i < vec->iovcnt; i++) {
        total += vec->array[i].iov_len;
        int nwritten = printf("   vec %3d base %p length %5zu total %5zu\n", i, (void *)  vec->array[i].iov_base, vec->array[i].iov_len, total);
        assertTrue(nwritten >= 0, "Error calling printf");
        longBowDebug_MemoryDump(vec->array[i].iov_base, vec->array[i].iov_len);
    }
}

size_t
ccnxCodecNetworkBufferIoVec_Length(const CCNxCodecNetworkBufferIoVec *vec)
{
    assertNotNull(vec, "Parameter vec must be non-null");
    return vec->totalBytes;
}

bool
ccnxCodecNetworkBufferIoVec_Equals(const CCNxCodecNetworkBufferIoVec *a, const CCNxCodecNetworkBufferIoVec *b)
{
    if (a == NULL && b == NULL) {
        return true;
    }

    if (a == NULL || b == NULL) {
        return false;
    }

    // both are non-null
    bool equals = false;
    if (a->totalBytes == b->totalBytes) {
        PARCBuffer *abuffer = ccnxCodecNetworkBuffer_CreateParcBuffer(a->networkBuffer);
        PARCBuffer *bbuffer = ccnxCodecNetworkBuffer_CreateParcBuffer(b->networkBuffer);

        equals = parcBuffer_Equals(abuffer, bbuffer);

        parcBuffer_Release(&abuffer);
        parcBuffer_Release(&bbuffer);
    }
    return equals;
}
