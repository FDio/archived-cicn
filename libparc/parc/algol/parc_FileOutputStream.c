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
#include <unistd.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_FileOutputStream.h>
#include <parc/algol/parc_Object.h>

PARCOutputStreamInterface *PARCFileOutputStreamAsPARCInputStream = &(PARCOutputStreamInterface) {
    .Acquire = (PARCOutputStream * (*)(PARCOutputStream *))parcFileOutputStream_Acquire,
    .Release = (void (*)(PARCOutputStream **))parcFileOutputStream_Release,
    .Write = (size_t (*)(PARCOutputStream *, PARCBuffer *))parcFileOutputStream_Write
};

struct parc_file_output_stream {
    int fd;
};

static void
_destroy(PARCFileOutputStream **streamPtr)
{
    PARCFileOutputStream *stream = *streamPtr;

    close(stream->fd);
}

parcObject_ExtendPARCObject(PARCFileOutputStream, _destroy, NULL, NULL, NULL, NULL, NULL, NULL);

PARCFileOutputStream *
parcFileOutputStream_Create(int fileDescriptor)
{
    assertTrue(fileDescriptor != -1, "Invalid file descriptor");

    PARCFileOutputStream *result = parcObject_CreateInstance(PARCFileOutputStream);
    result->fd = fileDescriptor;

    return result;
}

PARCOutputStream *
parcFileOutputStream_AsOutputStream(PARCFileOutputStream *fileOutputStream)
{
    return parcOutputStream_Create(parcFileOutputStream_Acquire(fileOutputStream), PARCFileOutputStreamAsPARCInputStream);
}

parcObject_ImplementAcquire(parcFileOutputStream, PARCFileOutputStream);

parcObject_ImplementRelease(parcFileOutputStream, PARCFileOutputStream);

bool
parcFileOutputStream_Write(PARCFileOutputStream *outputStream, PARCBuffer *buffer)
{
    const size_t maximumChunkSize = 1024 * 1024;

    while (parcBuffer_HasRemaining(buffer)) {
        size_t remaining = parcBuffer_Remaining(buffer);
        size_t chunkSize = remaining > maximumChunkSize ? maximumChunkSize : remaining;
        void *buf = parcBuffer_Overlay(buffer, chunkSize);
        ssize_t nwritten = write(outputStream->fd, buf, chunkSize);
        if (nwritten == -1) {
            break;
        }
    }

    return parcBuffer_HasRemaining(buffer) == false;
}
