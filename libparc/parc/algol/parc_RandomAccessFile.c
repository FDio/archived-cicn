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

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>

#include <parc/algol/parc_RandomAccessFile.h>

#include <stdio.h>

struct PARCRandomAccessFile {
    char *fname;
    FILE *fhandle;
};

static void
_parcRandomAccessFile_Finalize(PARCRandomAccessFile **instancePtr)
{
    parcAssertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCRandomAccessFile pointer.");
    PARCRandomAccessFile *instance = *instancePtr;
    if (instance->fhandle != NULL) {
        fclose(instance->fhandle);
    }
    if (instance->fname != NULL) {
        parcMemory_Deallocate(&instance->fname);
    }
}

parcObject_ImplementAcquire(parcRandomAccessFile, PARCRandomAccessFile);

parcObject_ImplementRelease(parcRandomAccessFile, PARCRandomAccessFile);

parcObject_ExtendPARCObject(PARCRandomAccessFile, _parcRandomAccessFile_Finalize, NULL,
                            parcRandomAccessFile_ToString, parcRandomAccessFile_Equals, NULL,
                            parcRandomAccessFile_HashCode, parcRandomAccessFile_ToJSON);


void
parcRandomAccessFile_AssertValid(const PARCRandomAccessFile *instance)
{
    parcAssertTrue(parcRandomAccessFile_IsValid(instance),
               "PARCRandomAccessFile is not valid.");
}

void
parcRandomAccessFile_Display(const PARCRandomAccessFile *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCRandomAccessFile@%p {", instance);
    parcDisplayIndented_PrintLine(indentation + 1, "File: %s", instance->fname);
    parcDisplayIndented_PrintLine(indentation, "}");
}

PARCHashCode
parcRandomAccessFile_HashCode(const PARCRandomAccessFile *instance)
{
    return parcHashCode_Hash((uint8_t *) instance->fname, strlen(instance->fname));
}

bool
parcRandomAccessFile_Equals(const PARCRandomAccessFile *x, const PARCRandomAccessFile *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        if (strcmp(x->fname, y->fname) == 0) {
            result = true;
        }
    }

    return result;
}

bool
parcRandomAccessFile_IsValid(const PARCRandomAccessFile *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = instance->fhandle != NULL;
    }

    return result;
}

PARCJSON *
parcRandomAccessFile_ToJSON(const PARCRandomAccessFile *instance)
{
    PARCJSON *result = parcJSON_Create();

    if (result != NULL) {
        parcJSON_AddString(result, "fname", instance->fname);
    }

    return result;
}

char *
parcRandomAccessFile_ToString(const PARCRandomAccessFile *instance)
{
    char *result = parcMemory_Format("PARCRandomAccessFile[%s]@%p\n", instance->fname, instance);
    return result;
}

PARCRandomAccessFile *
parcRandomAccessFile_Open(PARCFile *file)
{
    PARCRandomAccessFile *handle = parcObject_CreateAndClearInstance(PARCRandomAccessFile);
    if (handle != NULL) {
        char *fname = parcFile_ToString(file);
        handle->fhandle = fopen(fname, "r+");
        handle->fname = parcMemory_StringDuplicate(fname, strlen(fname));
        parcMemory_Deallocate(&fname);
    }
    return handle;
}

bool
parcRandomAccessFile_Close(PARCRandomAccessFile *fileHandle)
{
    parcAssertNotNull(fileHandle->fhandle, "Can't fclose a null pointer. How did they get one anyway?");
    bool result = fclose(fileHandle->fhandle) == 0;
    fileHandle->fhandle = NULL;
    parcMemory_Deallocate(&fileHandle->fname);
    fileHandle->fname = NULL;
    return result;
}

// read (count * size) bytes into the provided buffer, and return the number of bytes actually read
size_t
parcRandomAccessFile_Read(PARCRandomAccessFile *fileHandle, PARCBuffer *buffer)
{
    size_t length = parcBuffer_Remaining(buffer);
    size_t numBytes = fread(parcBuffer_Overlay(buffer, length), 1, length, fileHandle->fhandle);
    return numBytes;
}

// write (count * size) bytes from `buffer` to the file, and return the number of bytes actually written
size_t
parcRandomAccessFile_Write(PARCRandomAccessFile *fileHandle, PARCBuffer *buffer)
{
    size_t length = parcBuffer_Remaining(buffer);
    size_t numBytes = fwrite(parcBuffer_Overlay(buffer, length), 1, length, fileHandle->fhandle);
    return numBytes;
}

size_t
parcRandomAccessFile_Seek(PARCRandomAccessFile *fileHandle, long offset, PARCRandomAccessFilePosition position)
{
    size_t result = 0;
    switch (position) {
        case PARCRandomAccessFilePosition_Start:
            result = fseek(fileHandle->fhandle, offset, SEEK_SET); // beginning of the file
            break;
        case PARCRandomAccessFilePosition_Current:
            result = fseek(fileHandle->fhandle, offset, SEEK_CUR); // current offset
            break;
        case PARCRandomAccessFilePosition_End:
            result = fseek(fileHandle->fhandle, offset, SEEK_END); // end of the file
            break;
        default:
            parcAssertTrue(false, "Invalid position %d", position);
    }
    return result;
}
