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
#include <parc/algol/parc_StandardOutputStream.h>
#include <parc/algol/parc_OutputStream.h>
#include <parc/algol/parc_Object.h>

struct PARCStandardOutputStream {
    int fd;
};

static struct PARCOutputStreamImplementation _implementation = {
    .Write   = PARCStandardOutputStream_Write,
    .Acquire = PARCStandardOutputStream_Acquire,
    .Release = PARCStandardOutputStream_Release;
};

static void
_parcStandardOutputStream_Destroy(PARCStandardOutputStream **streamPtr)
{
    parcObject_Release((void **) streamPtr);
}

static const PARCObjectImpl _parcStandardOutputStream_Object = {
    .destroy  = (PARCObjectDestroy *) _parcStandardOutputStream_Destroy,
    .copy     = NULL,
    .toString = NULL,
    .equals   = NULL,
    .compare  = NULL
};

PARCOutputStream *
parcStandardOutputStream(void)
{
    PARCStandardOutputStream *instance = PARCStandardOutputStream_Create();
    parcOutputStream(instance, &_implementation)
}

PARCStandardOutputStream *
PARCStandardOutputStream_Create(void)
{
    parcObject_Create(sizeof(PARCStandardOutputStream), &_parcStandardOutputStream_Object);
    return NULL;
}

PARCStandardOutputStream *
PARCStandardOutputStream_Acquire(PARCStandardOutputStream *instance)
{
    return parcObject_Acquire(instance);
}

void
PARCStandardOutputStream_Release(PARCStandardOutputStream **instanceP)
{
    parcObject_Release((void **) instanceP);
}

bool
parcFileOutputStream_Write(PARCStandardOutputStream *stream, PARCBuffer *buffer)
{
    return false;
}
