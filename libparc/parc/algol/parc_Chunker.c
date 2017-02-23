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
#include <string.h>
#include <sys/time.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>

#include <parc/algol/parc_Chunker.h>

struct parc_chunker {
    PARCObject *instance;
    const PARCChunkerInterface *interface;
};

static void
_destroy(PARCChunker **chunkerP)
{
    parcObject_Release(&(*chunkerP)->instance);
}


parcObject_ExtendPARCObject(PARCChunker, _destroy, NULL, NULL, NULL, NULL, NULL, NULL);
parcObject_ImplementAcquire(parcChunker, PARCChunker);
parcObject_ImplementRelease(parcChunker, PARCChunker);

PARCChunker *
parcChunker_Create(PARCObject *instance, PARCChunkerInterface *interface)
{
    PARCChunker *chunker = parcObject_CreateInstance(PARCChunker);

    if (chunker != NULL) {
        chunker->instance = parcObject_Acquire(instance);
        chunker->interface = interface;
    }

    return chunker;
}

PARCIterator *
parcChunker_ForwardIterator(const PARCChunker *chunker)
{
    return (chunker->interface)->ForwardIterator(chunker->instance);
}

PARCIterator *
parcChunker_ReverseIterator(const PARCChunker *chunker)
{
    return (chunker->interface)->ReverseIterator(chunker->instance);
}

size_t
parcChunker_GetChunkSize(const PARCChunker *chunker)
{
    return (chunker->interface)->GetChunkSize(chunker->instance);
}
