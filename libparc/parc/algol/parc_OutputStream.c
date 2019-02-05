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
#include <stdarg.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_OutputStream.h>

struct parc_output_stream {
    void *instance;
    const PARCOutputStreamInterface *interface;
};

static void
_destroy(PARCOutputStream **streamPtr)
{
    PARCOutputStream *stream = *streamPtr;
    (stream->interface->Release)((PARCOutputStream **) &stream->instance);
}

parcObject_ExtendPARCObject(PARCOutputStream, _destroy, NULL, NULL, NULL, NULL, NULL, NULL);

PARCOutputStream *
parcOutputStream_Create(void *instance, const PARCOutputStreamInterface *interface)
{
    PARCOutputStream *result = parcObject_CreateInstance(PARCOutputStream);
    result->instance = instance;
    result->interface = interface;

    return result;
}

parcObject_ImplementAcquire(parcOutputStream, PARCOutputStream);

parcObject_ImplementRelease(parcOutputStream, PARCOutputStream);

size_t
parcOutputStream_Write(PARCOutputStream *stream, PARCBuffer *buffer)
{
    return (stream->interface->Write)(stream->instance, buffer);
}

size_t
parcOutputStream_WriteCStrings(PARCOutputStream *stream, ...)
{
    va_list ap;
    va_start(ap, stream);

    size_t result = 0;

    for (char *string = va_arg(ap, char *); string != NULL; string = va_arg(ap, char *)) {
        result += parcOutputStream_WriteCString(stream, string);
    }

    return result;
}

size_t
parcOutputStream_WriteCString(PARCOutputStream *stream, const char *string)
{
    PARCBuffer *buffer = parcBuffer_WrapCString((char *) string);
    size_t result = (stream->interface->Write)(stream->instance, buffer);
    parcBuffer_Release(&buffer);
    return result;
}
