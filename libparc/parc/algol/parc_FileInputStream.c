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

#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/stat.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_File.h>
#include <parc/algol/parc_Buffer.h>
#include <parc/algol/parc_InputStream.h>
#include <parc/algol/parc_FileInputStream.h>
#include <parc/algol/parc_Object.h>

PARCInputStreamInterface *PARCFileInputStreamAsPARCInputStream = &(PARCInputStreamInterface) {
    .Acquire = (PARCInputStream * (*)(const PARCInputStream *))parcFileInputStream_Acquire,
    .Release = (void (*)(PARCInputStream **))parcFileInputStream_Release,
    .Read = (size_t (*)(PARCInputStream *, PARCBuffer *))parcFileInputStream_Read
};

struct parc_file_input_stream {
    int fd;
};

static void
_destroy(PARCFileInputStream **inputStreamPtr)
{
    PARCFileInputStream *inputStream = *inputStreamPtr;

    close(inputStream->fd);
}

parcObject_ExtendPARCObject(PARCFileInputStream, _destroy, NULL, NULL, NULL, NULL, NULL, NULL);

PARCFileInputStream *
parcFileInputStream_Open(const PARCFile *file)
{
    PARCFileInputStream *result = NULL;

    char *fileName = parcFile_ToString(file);
    if (fileName != NULL) {
        result = parcFileInputStream_Create(open(fileName, O_RDONLY));
        parcMemory_Deallocate((void **) &fileName);
    }

    return result;
}

PARCFileInputStream *
parcFileInputStream_Create(int fileDescriptor)
{
    trapIllegalValueIf(fileDescriptor < 0, "File descriptor must not be negative.");

    PARCFileInputStream *result = parcObject_CreateInstance(PARCFileInputStream);
    if (result != NULL) {
        result->fd = fileDescriptor;
    }

    return result;
}

parcObject_ImplementAcquire(parcFileInputStream, PARCFileInputStream);
parcObject_ImplementRelease(parcFileInputStream, PARCFileInputStream);

bool
parcFileInputStream_Read(PARCFileInputStream *inputStream, PARCBuffer *buffer)
{
    while (parcBuffer_HasRemaining(buffer)) {
        void *buf = parcBuffer_Overlay(buffer, 0);
        ssize_t nread = read(inputStream->fd, buf, parcBuffer_Remaining(buffer));
        if (nread < 0) {
            break;
        }
        parcBuffer_SetPosition(buffer, parcBuffer_Position(buffer) + nread);
    }
    return parcBuffer_HasRemaining(buffer);
}

PARCBuffer *
parcFileInputStream_ReadFile(PARCFileInputStream *inputStream)
{
    PARCBuffer *result = NULL;

    struct stat statbuf;

    if (fstat(inputStream->fd, &statbuf) == 0) {
        result = parcBuffer_Allocate(statbuf.st_size);
        if (result != NULL) {
            parcFileInputStream_Read(inputStream, result);
        }
    }

    return result;
}
