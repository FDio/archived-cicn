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
 * @file codec/ccnxCodec_NetworkBuffer.h
 * @brief A network buffer represents memory used for network I/O
 *
 * A network buffer represents memory used for network I/O and may be scatter/gather non-contiguous memory or
 * may be made up for special memory regions, such as DMA memory directly from the kernel.
 *
 * The general usage pattern of this is to create the network buffer, fill it in with the encoded packet, then
 * create a CCNxCodecNetworkBufferIoVec from it.  The IoVec is then used in a gathering write.  Calling ccnxCodecNetworkBuffer_CreateIoVec()
 * will create a CCNxCodecNetworkBufferIoVec object that has a reference to the original network buffer and will release it when the
 * IoVec is released.  A user can get a normal system "struct iovec" from the CCNxCodecNetworkBufferIoVec.
 *
 * The CCNxCodecNetworkBufferIoVec is a read-only object.
 *
 * A network buffer uses a CCNxCodecNetworkBufferMemoryBlockFunctions structure for an allocator and de-allocator.  The allocator is called
 * to add more memory to the scatter/gather list of memory buffers and the de-allocator is used to return those
 * buffers to the owner.  A user could point to "ParcMemoryMemoryBlock" to use the normal parcMemory_allocate() and
 * parcMemory_deallocate() functions.  Or, they can use their own or wrap event buffers or wrap kernel memory blocks.
 *
 * The user can address the memory using a linearized position with ccnxCodecNetworkBuffer_Position() and ccnxCodecNetworkBuffer_SetPosition().
 * If a write would span two (or more) memory blocks, the write function will correctly split the write.
 *
 * When doing a write that would span two memory blocks, the network buffer may choose to truncate the current block and do an
 * unsplit write to the second block.  It will only do this if it would result in a small amount of wasted memory.  This can only
 * be done on the first pass through a memory block (if you set the position backwards and do a write that splits over memory blocks,
 * the write must be split).
 *
 * Add a control to turn off the "optimized" write splitting (i.e. the behavior to truncate the current block and do an unsplit
 * write to the next block).
 *
 * ccnxCodecNetworkBuffer_ComputeSignature should be factored out of here, like the verifier is factored out
 *
 */
#ifndef Libccnx_codec_ccnxCodecNetworkBuffer_h
#define Libccnx_codec_ccnxCodecNetworkBuffer_h

#include <stdint.h>
#include <sys/uio.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/security/parc_Signer.h>

struct ccnx_codec_network_buffer;
/**
 * @typedef CCNxCodecNetworkBuffer
 * @brief  A network buffer represents memory used for network I/O.
 */
typedef struct ccnx_codec_network_buffer CCNxCodecNetworkBuffer;

struct ccnx_codec_network_buffer_iovec;
/**
 * @typedef CCNxCodecNetworkBufferIoVec
 * @brief Contains a sequence of buffers to read in which the data to be read is stored.
 */
typedef struct ccnx_codec_network_buffer_iovec CCNxCodecNetworkBufferIoVec;

/**
 * @typedef CCNxCodecNetworkBufferMemoryBlockFunctions
 * @brief Structure and functions for MemoryBlocks
 */

typedef struct ccnx_codec_network_buffer_memory_block_struct {
    /**
     * Allocate a block of memory at least 'bytes' long.
     *
     * @param [in] userarg Closure, may be null.
     * @param [in] bytes The requested number of bytes.
     * @param [out] output The buffer to set.
     *
     * @return The number of bytes granted in output.
     */
    size_t (*allocator)(void *userarg, size_t bytes, void **output);

    /**
     * Returns (frees) a memory block.
     *
     * @param [in] userarg Closure, may be null.
     * @param [out] memoryPtr The memory pointer to dellocate and NULLify.
     */
    void (*deallocator)(void *userarg, void **memoryPtr);
} CCNxCodecNetworkBufferMemoryBlockFunctions;

extern const CCNxCodecNetworkBufferMemoryBlockFunctions ParcMemoryMemoryBlock;

/**
 * Creates a `CCNxCodecNetworkBuffer`.
 *
 * The first memory block is allocated using the default settings.  The parameter "userarg" will be passed
 * to the CCNxCodecNetworkBufferMemoryBlockFunctions for allocations and de-allocations.
 *
 * @param [in] blockFunctions The allocator/de-allocator to use.
 * @param [in] userarg Passed to all calls to the blockFunctions, may be NULL.
 *
 * @return non-null An allocated memory block using memory from blockFunctions.
 * @return null An error
 *
 * Example:
 * @code
 * {
 *     CCNxCodecNetworkBuffer * netbuffer = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
 * }
 * @endcode
 */
CCNxCodecNetworkBuffer *ccnxCodecNetworkBuffer_Create(const CCNxCodecNetworkBufferMemoryBlockFunctions *blockFunctions, void *userarg);

/**
 * Create a `CCNxCodecNetworkBuffer` from a buffer block.
 *
 * The first memory block of the Network Buffer will wrap the user provided memory.
 *
 * If the allocator is non-null then the user could append more memory blocks.
 *
 * The deallocator in the blockFunctions will be called on the memory when done.  The userarg will
 * be passed to the CCNxCodecNetworkBufferMemoryBlockFunctions.
 *
 * @param [in] blockFunctions The allocator/de-allocator to use.
 * @param [in] userarg Passed to all calls to the blockFunctions, may be NULL.
 * @param [in] length The length of the user-provided memory.
 * @param [in] memory The user-provided memory.
 *
 * @return non-null An allocated memory block that wraps the user-provided memory.
 * @return null An error occurred.
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 * }
 * @endcode
 */
CCNxCodecNetworkBuffer *ccnxCodecNetworkBuffer_CreateFromArray(const CCNxCodecNetworkBufferMemoryBlockFunctions *blockFunctions, void *userarg, size_t length, uint8_t *memory);

/**
 * Increase the number of references to a `CCNxCodecNetworkBuffer`.
 *
 * Note that new `CCNxCodecNetworkBuffer` is not created,
 * only that the given `CCNxCodecNetworkBuffer` reference count is incremented.
 * Discard the reference by invoking `ccnxCodecNetworkBuffer_Release`.
 *
 * @param [in] original A pointer to a `CCNxCodecNetworkBuffer` instance.
 *
 * @return The input `CCNxCodecNetworkBuffer` pointer.
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     CCNxCodecNetworkBuffer *handle = ccnxCodecNetworkBuffer_Acquire(netbuff);
 *
 *     ...
 *
 *     ccnxCodecNetworkBuffer_Release(&handle);
 *     ccnxCodecNetworkBuffer_Release(&netbuff);
 * }
 * @endcode
 */
CCNxCodecNetworkBuffer *ccnxCodecNetworkBuffer_Acquire(CCNxCodecNetworkBuffer *original);

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
 * @param [in,out] bufferPtr A pointer to a pointer to the instance to release.
 *
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     CCNxCodecNetworkBuffer *handle = ccnxCodecNetworkBuffer_Acquire(netbuff);
 *
 *     ...
 *
 *     ccnxCodecNetworkBuffer_Release(&handle);
 *     ccnxCodecNetworkBuffer_Release(&netbuff);
 * }
 * @endcode
 */
void ccnxCodecNetworkBuffer_Release(CCNxCodecNetworkBuffer **bufferPtr);

/**
 * Returns the linearlized cursor position in the buffer.
 *
 * Returns the current cursor position in linearized memory location (this does not
 * actually linearize the memory).
 *
 * @param [in] buffer An allocated network buffer.
 *
 * @return The linearized memory position (bytes).
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     size_t position = ccnxCodecNetworkBuffer_Position(netbuff);
 *     // position is 0 since nothing has been written yet
 * }
 * @endcode
 */
size_t ccnxCodecNetworkBuffer_Position(const CCNxCodecNetworkBuffer *buffer);

/**
 * Returns the maximum position to which is written.
 *
 * The maximum position of the currently written memory, as if it were linear memory. The limit
 * will be "0" if no data has been written.
 *
 * @param [in] buffer An allocated network buffer.
 *
 * @return The lineaized capacity (bytes).
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     size_t limit = ccnxCodecNetworkBuffer_Limit(netbuff);
 *     // limit is msg_length since the buffer was created as a wrapper for the packet_buffer
 * }
 * @endcode
 */
size_t ccnxCodecNetworkBuffer_Limit(const CCNxCodecNetworkBuffer *buffer);

/**
 * Sets the cursor position to the linearized memory location.
 *
 * Sets the cursor to the linearized memory location. It must not exceed {@link ccnxCodecNetworkBuffer_Limit}().
 *
 * @param [in,out] buffer An allocated `CCNxCodecNetworkBuffer`.
 * @param [in] position The linearized buffer position.
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     ccnxCodecNetworkBuffer_SetPosition(netbuff, 1);
 *     // the position is now 1, instead of 0
 * }
 * @endcode
 */
void ccnxCodecNetworkBuffer_SetPosition(CCNxCodecNetworkBuffer *buffer, size_t position);

/**
 * Sets the buffer limit to the current position.  Throws away anything after.
 *
 * The Limit will be set to the current position.  Any bytes left after the new Limit are discarded
 * and un-recoverable.  This should be done after finishing writing to the buffer, especially if
 * the buffer was backed up to discard or overwrite previous data.
 *
 * @param [in,out] buffer An allocated `CCNxCodecNetworkBuffer`.
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     size_t newPosition = ccnxCodecNetworkBuffer_Position(netbuff) + sizeof(uint16_t);
 *     ccnxCodecNetworkBuffer_PutUint16(netbuff, 0x1234);
 *     ccnxCodecNetworkBuffer_SetPosition(netbuff, 1);
 *     ccnxCodecNetworkBuffer_Finalize(netbuff);
 *     // the buffer is now only '0x12' and the limit is reduced to 1.
 * }
 * @endcode
 */
void ccnxCodecNetworkBuffer_Finalize(CCNxCodecNetworkBuffer *buffer);

/**
 * Writes a `uint8_t` to the current cursor position, allocating as necessary
 *
 * Writes to the current cursor position. If the cursor is at the end of a memory block,
 * a new memory block will be allocated. Will assert if cannot allocate more memory (or if the allocator is null).
 *
 * @param [in,out] buffer An allocated `CCNxCodecNetworkBuffer`.
 * @param [in] value The value to write.
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     size_t newPosition = ccnxCodecNetworkBuffer_Position(netbuff) + sizeof(uint8_t);
 *     ccnxCodecNetworkBuffer_PutUint8(netbuff, 0x12);
 *
 *     size_t position = ccnxCodecNetworkBuffer_Position(netbuff);
 *     // position will equal newPosition
 * }
 * @endcode
 */
void ccnxCodecNetworkBuffer_PutUint8(CCNxCodecNetworkBuffer *buffer, uint8_t value);

/**
 * Writes a `uint16_t` to the current cursor position, allocating as necessary.
 *
 * Writes to the current cursor position. If the cursor is at the end of a memory block,
 * a new memory block will be allocated. Will assert if cannot allocate more memory (or if the allocator is null).
 * The value is written in network byte order.
 *
 * @param [in,out] buffer An allocated `CCNxCodecNetworkBuffer`.
 * @param [in] value The value to write.
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     size_t newPosition = ccnxCodecNetworkBuffer_Position(netbuff) + sizeof(uint16_t);
 *     ccnxCodecNetworkBuffer_PutUint16(netbuff, 0x1234);
 *
 *     size_t position = ccnxCodecNetworkBuffer_Position(netbuff);
 *     // position will equal newPosition
 * }
 * @endcode
 */
void ccnxCodecNetworkBuffer_PutUint16(CCNxCodecNetworkBuffer *buffer, uint16_t value);

/**
 * Writes a `uint32_t` to the current cursor position, allocating as necessary.
 *
 * Writes to the current cursor position. If the cursor is at the end of a memory block,
 * a new memory block will be allocated. Will assert if cannot allocate more memory (or if the allocator is null).
 * The value is written in network byte order.
 *
 * @param [in,out] buffer An allocated `CCNxCodecNetworkBuffer`.
 * @param [in] value The value to write.
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     size_t newPosition = ccnxCodecNetworkBuffer_Position(netbuff) + sizeof(uint32_t);
 *     ccnxCodecNetworkBuffer_PutUint32(netbuff, 0x12345678);
 *
 *     size_t position = ccnxCodecNetworkBuffer_Position(netbuff);
 *     // position will equal newPosition
 * }
 * @endcode
 */
void ccnxCodecNetworkBuffer_PutUint32(CCNxCodecNetworkBuffer *buffer, uint32_t value);

/**
 * Writes a `uint64_t` to the current cursor position, allocating as necessary
 *
 * Writes to the current cursor position. If the cursor is at the end of a memory block,
 * a new memory block will be allocated. Will assert if cannot allocate more memory (or if the allocator is null).
 * The value is written in network byte order.
 *
 * @param [in,out] buffer An allocated `CCNxCodecNetworkBuffer`.
 * @param [in] value The value to write.
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     size_t newPosition = ccnxCodecNetworkBuffer_Position(netbuff) + sizeof(uint64_t);
 *     ccnxCodecNetworkBuffer_PutUint64(netbuff, 0x1234567812345678);
 *
 *     size_t position = ccnxCodecNetworkBuffer_Position(netbuff);
 *     // position will equal newPosition
 * }
 * @endcode
 */
void ccnxCodecNetworkBuffer_PutUint64(CCNxCodecNetworkBuffer *buffer, uint64_t value);

/**
 * Writes an array to the current cursor position, allocating as necessary
 *
 * Writes to the current cursor position. If the cursor is at the end of a memory block,
 * a new memory block will be allocated. Will assert if cannot allocate more memory (or if the allocator is null).
 * The value is written in array order.
 *
 * @param [in,out] buffer An allocated `CCNxCodecNetworkBuffer`.
 * @param [in] length AThe length of the array to write.
 * @param [in] array The value to write.
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     uint8_t array[5] = {1,2,3,4,5};
 *     size_t newPosition = ccnxCodecNetworkBuffer_Position(netbuff) + 5;
 *     ccnxCodecNetworkBuffer_PutArray(netbuff, array, 5);
 *
 *     size_t position = ccnxCodecNetworkBuffer_Position(netbuff);
 *     // position will equal newPosition
 * }
 * @endcode
 */
void ccnxCodecNetworkBuffer_PutArray(CCNxCodecNetworkBuffer *buffer, size_t length, const uint8_t *array);

/**
 * Writes to the current cursor position, allocating as necessary
 *
 * Writes to the current cursor position. If the cursor is at the end of a memory block,
 * a new memory block will be allocated. Will assert if cannot allocate more memory (or if the allocator is null).
 * The value is written in buffer order.
 *
 * @param [in,out] buffer An allocated `CCNxCodecNetworkBuffer`.
 * @param [in] value The value to write.
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     uint8_t array[10] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9 };
 *     PARCBuffer *buffer = parcBuffer_Wrap(array, 10, 0, 10);
 *     size_t newPosition = ccnxCodecNetworkBuffer_Position(netbuff) + 10;
 *     ccnxCodecNetworkBuffer_PutBuffer(netbuff, buffer);
 *
 *     size_t position = ccnxCodecNetworkBuffer_Position(netbuff);
 *     // position will equal newPosition
 * }
 * @endcode
 */
void ccnxCodecNetworkBuffer_PutBuffer(CCNxCodecNetworkBuffer *buffer, PARCBuffer *value);

/**
 * Creates a linearized memory buffer.
 *
 * Allocates a single buffer and copies the `CCNxCodecNetworkBuffer` to it.
 *
 * @param [in] buffer An allocated `CCNxCodecNetworkBuffer`.
 *
 * @return non-null A copy of the network buffer's written contents.
 * @return null An error.
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     size_t newPosition = ccnxCodecNetworkBuffer_Position(netbuff) + sizeof(uint8_t);
 *     ccnxCodecNetworkBuffer_PutUint8(netbuff, 0x12);
 *
 *     PARCBuffer *buffer = ccnxCodecNetworkBuffer_CreateParcBuffer(netbuff);
 * }
 * @endcode
 */
PARCBuffer *ccnxCodecNetworkBuffer_CreateParcBuffer(CCNxCodecNetworkBuffer *buffer);

/**
 * Runs a signer over the network buffer
 *
 * Runs a {@link PARCSigner} over a specified range of the network buffer
 *
 * @param [in] buffer An allocated `CCNxCodecNetworkBuffer`.
 * @param [in] start The start position (must be 0 <= start < Limit)
 * @param [in] end The last posiiton (start < end <= Limit)
 * @param [in] signer The `PARCSigner`
 *
 * @return non-null The {@link PARCSignature} computed by the signer
 * @return null An error
 *
 * Example:
 * @code
 * {
 *     parcSecurity_Init();
 *     PARCSigner *signer = parcSigner_Create(parcPublicKeySignerPkcs12Store_Open("test_rsa.p12", "blueberry", PARCCryptoHashType_SHA256));
 *
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     size_t newPosition = ccnxCodecNetworkBuffer_Position(netbuff) + sizeof(uint8_t);
 *     ccnxCodecNetworkBuffer_PutUint8(netbuff, 0x12);
 *
 *     PARCSignature *sig = ccnxCodecNetworkBuffer_ComputeSignature(netbuff, 0, ccnxCodecNetworkBuffer_Limit(netbuff), signer);
 * }
 * @endcode
 */
PARCSignature *ccnxCodecNetworkBuffer_ComputeSignature(CCNxCodecNetworkBuffer *buffer, size_t start, size_t end, PARCSigner *signer);

/**
 * Get a `uint8_t` byte from the buffer, does not change position.
 *
 * Reads the byte at the given position. The position must be less than the buffer's limit.
 *
 * @param [in] netbuff An allocated memory buffer.
 * @param [in] position Must be 0 <= position < Limit.
 *
 * @return number The specified byte
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     size_t newPosition = ccnxCodecNetworkBuffer_Position(netbuff) + sizeof(uint8_t);
 *     ccnxCodecNetworkBuffer_PutUint8(netbuff, 0x12);
 *
 *     uint8_t byte = ccnxCodecNetworkBuffer_GetUint8(netbuff, 0);
 * }
 * @endcode
 */
uint8_t ccnxCodecNetworkBuffer_GetUint8(const CCNxCodecNetworkBuffer *netbuff, size_t position);

/**
 * Prints the buffer to the console.
 *
 * @param [in] netbuff A `CCNxCodecNetworkBuffer` instance.
 * @param [in] indent The number of spaces by which to indent the output.
 *
 * Example:
 * @code
 * {
 *     uint8_t *packet_buffer;
 *     packet_buffer = parcMemory_Allocate(msg_length);
 *     read(fd, packet_buffer, msg_length);
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_CreateFromArray(&ParcMemoryMemoryBlock, NULL, msg_length, packet_buffer);
 *
 *     size_t newPosition = ccnxCodecNetworkBuffer_Position(netbuff) + sizeof(uint8_t);
 *     ccnxCodecNetworkBuffer_PutUint8(netbuff, 0x12);
 *
 *     ccnxCodecNetworkBuffer_Display(netbuff, 0);
 * }
 * @endcode
 */
void ccnxCodecNetworkBuffer_Display(const CCNxCodecNetworkBuffer *netbuff, unsigned indent);

// ======================================================================
// IoVec related

/**
 * Creates a read-only `CCNxCodecNetworkBufferIoVec` representation of the `CCNxCodecNetworkBuffer`.
 *
 * Creates a reference to the `CCNxCodecNetworkBuffer`, so the buffer will not go away until the IoVec is released.
 * It is a zero-copy operation.  The IoVec is a read-only representation.  It is used to return a "struct iovec"
 * for doing a gathering write.
 *
 * @param [in] buffer An allocated `CCNxCodecNetworkBuffer` (will acquire a reference to it).
 *
 * @return non-null An allocated {@link CCNxCodecNetworkBufferIoVec}, you must call {@link ccnxCodecNetworkBufferIoVec_Release}
 * @return null An error
 *
 * Example:
 * @code
 * {
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
 *     ccnxCodecNetworkBuffer_PutArray(netbuff, sizeof(interest_nameA), interest_nameA);
 *     CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);
 *     ccnxCodecNetworkBuffer_Release(&netbuff);
 *
 *     // the memory represented by iov will be "interest_nameA"
 *     const struct iovec * iov = ccnxCodecNetworkBufferIoVec_GetArray(vec);
 *
 *     // final release of "netbuff" too
 *     ccnxCodecNetworkBufferIoVec_Release(&vec);
 * }
 * @endcode
 */
CCNxCodecNetworkBufferIoVec *ccnxCodecNetworkBuffer_CreateIoVec(CCNxCodecNetworkBuffer *buffer);

/**
 * Increase the number of references to a `CCNxCodecNetworkBufferIoVec`.
 *
 * Note that new `CCNxCodecNetworkBufferIoVec` is not created,
 * only that the given `CCNxCodecNetworkBufferIoVec` reference count is incremented.
 * Discard the reference by invoking `ccnxCodecNetworkBufferIoVec_Release`.
 *
 * @param [in] vec A pointer to a `CCNxCodecNetworkBufferIoVec` instance to acquire.
 *
 * @return The @p vec.
 *
 * Example:
 * @code
 * {
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
 *     ccnxCodecNetworkBuffer_PutArray(netbuff, sizeof(interest_nameA), interest_nameA);
 *     CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);
 *     ccnxCodecNetworkBuffer_Release(&netbuff);
 *
 *     CCNxCodecNetworkBufferIoVec *handle = ccnxCodecNetworkBufferIoVec_Acquire(vec);
 *     const struct iovec *iov = ccnxCodecNetworkBufferIoVec_GetArray(handle);
 *
 *     ccnxCodecNetworkBufferIoVec_Release(&handle);
 *     ccnxCodecNetworkBufferIoVec_Release(&vec);
 * }
 * @endcode
 */
CCNxCodecNetworkBufferIoVec *ccnxCodecNetworkBufferIoVec_Acquire(CCNxCodecNetworkBufferIoVec *vec);

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
 * @param [in,out] vecPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
 *     ccnxCodecNetworkBuffer_PutArray(netbuff, sizeof(interest_nameA), interest_nameA);
 *     CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);
 *     ccnxCodecNetworkBuffer_Release(&netbuff);
 *
 *     const struct iovec *iov = ccnxCodecNetworkBufferIoVec_GetArray(vec);
 *
 *     ccnxCodecNetworkBufferIoVec_Release(&vec);
 * }
 * @endcode
 */
void ccnxCodecNetworkBufferIoVec_Release(CCNxCodecNetworkBufferIoVec **vecPtr);

/**
 * Returns the number of extents in the iovec.
 *
 * The number of memory buffers gathered in the iovec.
 *
 * @param [in] vec An allocated `CCNxCodecNetworkBufferIoVec`.
 *
 * @return The number of buffers gathered in the iovec.
 *
 * Example:
 * @code
 * {
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
 *     ccnxCodecNetworkBuffer_PutArray(netbuff, sizeof(interest_nameA), interest_nameA);
 *     CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);
 *     ccnxCodecNetworkBuffer_Release(&netbuff);
 *
 *     // the memory represented by iov will be "interest_nameA"
 *     writev(fh, ccnxCodecNetworkBufferIoVec_GetArray(vec), ccnxCodecNetworkBufferIoVec_GetCount(vec));
 *
 *     // final release of "netbuff" too
 *     ccnxCodecNetworkBufferIoVec_Release(&vec);
 * }
 * @endcode
 */
int ccnxCodecNetworkBufferIoVec_GetCount(CCNxCodecNetworkBufferIoVec *vec);

/**
 * Returns an iovec representation of the memory.
 *
 * Returns the internal iovec representation of the memory.  You do NOT need to free this.
 *
 * @param [in] vec An allocated `CCNxCodecNetworkBufferIoVec`.
 *
 * @return non-null The internal iovec representation of the memory, do not free
 * @return null An error
 *
 * Example:
 * @code
 * {
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
 *     ccnxCodecNetworkBuffer_PutArray(netbuff, sizeof(interest_nameA), interest_nameA);
 *     CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);
 *     ccnxCodecNetworkBuffer_Release(&netbuff);
 *
 *     // the memory represented by iov will be "interest_nameA"
 *     writev(fh, ccnxCodecNetworkBufferIoVec_GetArray(vec), ccnxCodecNetworkBufferIoVec_GetCount(vec));
 *
 *     // final release of "netbuff" too
 *     ccnxCodecNetworkBufferIoVec_Release(&vec);
 * }
 * @endcode
 */
const struct iovec *ccnxCodecNetworkBufferIoVec_GetArray(CCNxCodecNetworkBufferIoVec *vec);

/**
 * Dispalys the CCNxCodecNetworkBufferIoVec to the console.
 *
 * @param [in] vec  A `CCNxCodecNetworkBufferIoVec` instance.
 * @param [in] indent The number of spaces by which to indent the output.
 *
 * Example:
 * @code
 * {
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
 *     ccnxCodecNetworkBuffer_PutArray(netbuff, sizeof(interest_nameA), interest_nameA);
 *     CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);
 *     ccnxCodecNetworkBuffer_Release(&netbuff);
 *
 *     ccnxCodecNetworkBufferIoVec_Display(vec, 0);
 *
 *     ccnxCodecNetworkBufferIoVec_Release(&vec);
 * }
 * @endcode
 */
void ccnxCodecNetworkBufferIoVec_Display(const CCNxCodecNetworkBufferIoVec *vec, int indent);

/**
 * The total bytes of all iovecs.
 *
 * The total number of bytes as if linearized memory.
 *
 * @param [in] vec An allocated `CCNxCodecNetworkBufferIoVec`.
 *
 * @return number The total bytes represented by all iovecs
 *
 * Example:
 * @code
 * {
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
 *     ccnxCodecNetworkBuffer_PutArray(netbuff, sizeof(interest_nameA), interest_nameA);
 *     CCNxCodecNetworkBufferIoVec *vec = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);
 *     ccnxCodecNetworkBuffer_Release(&netbuff);
 *
 *     size_t interestSize = ccnxCodecNetworkBufferIoVec_Length(vec);
 *     // interestSize will be the size of the linearized interest put into the network buffer
 *
 *     ccnxCodecNetworkBufferIoVec_Release(&vec);
 * }
 * @endcode
 */
size_t ccnxCodecNetworkBufferIoVec_Length(const CCNxCodecNetworkBufferIoVec *vec);

/**
 * Determine if two `CCNxCodecNetworkBufferIoVec` instances are equal.
 *
 * The following equivalence relations on non-null `CCNxCodecNetworkBufferIoVec` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, `ccnxCodecNetworkBufferIoVec_Equals(x, x)` must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, `ccnxCodecNetworkBufferIoVec_Equals(x, y)` must return true if and only if
 *        `ccnxCodecNetworkBufferIoVec_Equals(y x)` returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        `ccnxCodecNetworkBufferIoVec_Equals(x, y)` returns true and
 *        `ccnxCodecNetworkBufferIoVec_Equals(y, z)` returns true,
 *        then `ccnxCodecNetworkBufferIoVec_Equals(x, z)` must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of `ccnxCodecNetworkBufferIoVec_Equals(x, y)`
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, `ccnxCodecNetworkBufferIoVec_Equals(x, NULL)` must return false.
 *
 *
 * @param [in] a A pointer to a `CCNxCodecNetworkBufferIoVec` instance.
 * @param [in] b A pointer to a `CCNxCodecNetworkBufferIoVec` instance.
 *
 * @return true if `CCNxCodecNetworkBufferIoVec` x and y are equal.
 * @return false if `CCNxCodecNetworkBufferIoVec` x and y are not equal.
 *
 * Example:
 * @code
 * {
 *     CCNxCodecNetworkBuffer *netbuff = ccnxCodecNetworkBuffer_Create(&ParcMemoryMemoryBlock, NULL);
 *     ccnxCodecNetworkBuffer_PutArray(netbuff, sizeof(interest_nameA), interest_nameA);
 *     CCNxCodecNetworkBufferIoVec *vec1 = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);
 *     CCNxCodecNetworkBufferIoVec *vec2 = ccnxCodecNetworkBuffer_CreateIoVec(netbuff);
 *     ccnxCodecNetworkBuffer_Release(&netbuff);
 *
 *     if (ccnxCodecNetworkBufferIoVec_Equals(vec1, vec2)) {
 *         printf("IOVectors are equal.\n");
 *     } else {
 *          printf("IOVectors are NOT equal.\n");
 *      }
 *
 *     ccnxCodecNetworkBufferIoVec_Release(&vec);
 * }
 * @endcode
 */
bool ccnxCodecNetworkBufferIoVec_Equals(const CCNxCodecNetworkBufferIoVec *a, const CCNxCodecNetworkBufferIoVec *b);
#endif // Libccnx_codec_ccnxCodecNetworkBuffer_h
