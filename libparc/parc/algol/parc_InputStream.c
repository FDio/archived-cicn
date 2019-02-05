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

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_InputStream.h>

struct parc_input_stream {
    void *instance;
    const PARCInputStreamInterface *interface;
};

static void
_destroy(PARCInputStream **inputStreamPtr)
{
    PARCInputStream *inputStream = *inputStreamPtr;
    parcObject_Release(&inputStream->instance);
}

parcObject_ExtendPARCObject(PARCInputStream, _destroy, NULL, NULL, NULL, NULL, NULL, NULL);


PARCInputStream *
parcInputStream(void *instance, const PARCInputStreamInterface *interface)
{
    PARCInputStream *result = parcObject_CreateInstance(PARCInputStream);
    result->instance = instance;
    result->interface = interface;

    return result;
}

parcObject_ImplementAcquire(parcInputStream, PARCInputStream);

parcObject_ImplementRelease(parcInputStream, PARCInputStream);

size_t
parcInputStream_Read(PARCInputStream *inputStream, PARCBuffer *buffer)
{
    return (inputStream->interface->Read)(inputStream->instance, buffer);
}
