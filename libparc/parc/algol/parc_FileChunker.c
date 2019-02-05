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

#ifndef _WIN32
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_RandomAccessFile.h>
#include <parc/algol/parc_FileChunker.h>

PARCChunkerInterface *PARCFileChunkerAsChunker = &(PARCChunkerInterface) {
    .ForwardIterator = (void *(*)(const void *))parcFileChunker_ForwardIterator,
    .ReverseIterator = (void *(*)(const void *))parcFileChunker_ReverseIterator,
    .GetChunkSize = (size_t (*)(const void *))parcFileChunker_GetChunkSize
};

struct _parc_chunker_state {
    size_t chunkNumber;
    int direction;
    bool atEnd;
    size_t position;
    size_t nextChunkSize;
    size_t totalSize;
};

typedef struct _parc_chunker_state _ChunkerState;

struct parc_buffer_chunker {
    // State
    size_t chunkSize;

    // Container for the data to be parsed
    PARCFile *file;
    PARCRandomAccessFile *fhandle;

    // The current element of the iterator
    PARCBuffer *currentElement;
};

static void
_destroy(PARCFileChunker **chunkerP)
{
    if ((*chunkerP)->fhandle != NULL) {
        parcRandomAccessFile_Release(&(*chunkerP)->fhandle);
    }

    if ((*chunkerP)->file != NULL) {
        parcFile_Release(&(*chunkerP)->file);
    }

    if ((*chunkerP)->currentElement != NULL) {
        parcBuffer_Release(&(*chunkerP)->currentElement);
    }
}

static void *
_InitForward(PARCFileChunker *chunker)
{
    _ChunkerState *state = parcMemory_Allocate(sizeof(_ChunkerState));

    state->chunkNumber = 0;
    state->direction = 0;
    state->position = 0;
    state->atEnd = false;
    state->totalSize = parcFile_GetFileSize(chunker->file);

    if (state->totalSize < chunker->chunkSize) {
        state->position = 0;
        state->nextChunkSize = state->totalSize;
    } else {
        state->position = 0;
        state->nextChunkSize = chunker->chunkSize;
    }

    return state;
}

static void *
_InitReverse(PARCFileChunker *chunker)
{
    _ChunkerState *state = parcMemory_Allocate(sizeof(_ChunkerState));

    state->chunkNumber = 0;
    state->direction = 1;
    state->atEnd = false;
    state->totalSize = parcFile_GetFileSize(chunker->file);

    if (state->totalSize < chunker->chunkSize) {
        state->position = 0;
        state->nextChunkSize = state->totalSize;
    } else {
        state->position = state->totalSize - chunker->chunkSize;
        state->nextChunkSize = chunker->chunkSize;
    }

    return state;
}

static bool
_parcChunker_HasNext(PARCFileChunker *chunker, void *voidstate)
{
    _ChunkerState *state = (_ChunkerState *) voidstate;
    return !state->atEnd;
}

static void
_advanceStateForward(PARCFileChunker *chunker, _ChunkerState *state)
{
    state->position += state->nextChunkSize;
    size_t remaining = state->totalSize - state->position;

    if (remaining == 0) {
        state->atEnd = true;
    } else if (remaining > chunker->chunkSize) {
        state->nextChunkSize = chunker->chunkSize;
    } else {
        state->nextChunkSize = remaining;
    }
}

static void
_advanceStateBackward(PARCFileChunker *chunker, _ChunkerState *state)
{
    // Previous chunk size
    size_t chunkSize = state->nextChunkSize;
    if (chunkSize != chunker->chunkSize || state->position == 0) {
        state->atEnd = true;
    } else {
        if (state->position < chunkSize) {
            state->nextChunkSize = state->position; // on next read, go to the current position
            state->position = 0; // we reached the beginning
        } else {
            state->position -= chunkSize;
        }
    }
}

static void
_advanceState(PARCFileChunker *chunker, _ChunkerState *state)
{
    state->chunkNumber++;

    if (state->direction == 0) {
        _advanceStateForward(chunker, state);
    } else {
        _advanceStateBackward(chunker, state);
    }
}

static void *
_parcChunker_NextFromBuffer(PARCFileChunker *chunker, _ChunkerState *state)
{
    size_t chunkSize = state->nextChunkSize;

    parcRandomAccessFile_Seek(chunker->fhandle, (long)(state->position), PARCRandomAccessFilePosition_Start);

    PARCBuffer *slice = parcBuffer_Allocate(chunkSize);
    parcRandomAccessFile_Read(chunker->fhandle, slice);
    slice = parcBuffer_Flip(slice);

    _advanceState(chunker, state);

    return slice;
}

static void *
_parcChunker_Next(PARCFileChunker *chunker, void *state)
{
    PARCBuffer *buffer = _parcChunker_NextFromBuffer(chunker, state);

    if (chunker->currentElement != NULL) {
        parcBuffer_Release(&chunker->currentElement);
    }
    if (buffer != NULL) {
        chunker->currentElement = parcBuffer_Acquire(buffer);
    }

    return state;
}

static void
_parcChunker_RemoveAt(PARCFileChunker *chunker, void **state)
{
    // pass
}

static void *
_parcChunker_GetElement(PARCFileChunker *chunker, void *state)
{
    return chunker->currentElement;
}

static void
_parcChunker_Finish(PARCFileChunker *chunker, void *state)
{
    _ChunkerState *thestate = (_ChunkerState *) state;
    parcMemory_Deallocate(&thestate);
}

static void
_parcChunker_AssertValid(const void *state)
{
    // pass
}

parcObject_ExtendPARCObject(PARCFileChunker, _destroy, NULL, NULL, NULL, NULL, NULL, NULL);
parcObject_ImplementAcquire(parcFileChunker, PARCFileChunker);
parcObject_ImplementRelease(parcFileChunker, PARCFileChunker);

PARCFileChunker *
parcFileChunker_Create(PARCFile *file, size_t chunkSize)
{
    PARCFileChunker *chunker = parcObject_CreateInstance(PARCFileChunker);

    if (chunker != NULL) {
        chunker->chunkSize = chunkSize;
        chunker->file = parcFile_Acquire(file);
        chunker->fhandle = parcRandomAccessFile_Open(chunker->file);
        chunker->currentElement = NULL;
    }

    return chunker;
}


PARCIterator *
parcFileChunker_ForwardIterator(const PARCFileChunker *chunker)
{
    PARCIterator *iterator = parcIterator_Create((void *) chunker,
                                                 (void *(*)(PARCObject *))_InitForward,
                                                 (bool (*)(PARCObject *, void *))_parcChunker_HasNext,
                                                 (void *(*)(PARCObject *, void *))_parcChunker_Next,
                                                 (void (*)(PARCObject *, void **))_parcChunker_RemoveAt,
                                                 (void *(*)(PARCObject *, void *))_parcChunker_GetElement,
                                                 (void (*)(PARCObject *, void *))_parcChunker_Finish,
                                                 (void (*)(const void *))_parcChunker_AssertValid);

    return iterator;
}

PARCIterator *
parcFileChunker_ReverseIterator(const PARCFileChunker *chunker)
{
    PARCIterator *iterator = parcIterator_Create((void *) chunker,
                                                 (void *(*)(PARCObject *))_InitReverse,
                                                 (bool (*)(PARCObject *, void *))_parcChunker_HasNext,
                                                 (void *(*)(PARCObject *, void *))_parcChunker_Next,
                                                 (void (*)(PARCObject *, void **))_parcChunker_RemoveAt,
                                                 (void *(*)(PARCObject *, void *))_parcChunker_GetElement,
                                                 (void (*)(PARCObject *, void *))_parcChunker_Finish,
                                                 (void (*)(const void *))_parcChunker_AssertValid);

    return iterator;
}

size_t
parcFileChunker_GetChunkSize(const PARCFileChunker *chunker)
{
    return chunker->chunkSize;
}
