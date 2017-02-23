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

#include <LongBow/runtime.h>

#include <inttypes.h>
#include <stdarg.h>

#include <parc/logging/parc_LogEntry.h>

#include <parc/algol/parc_Time.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/logging/parc_LogLevel.h>

static const char _parcLog_Version = 1;

struct PARCLogEntry {
    PARCLogLevel level;
    char version;
    struct timeval timeStamp;
    char *hostName;
    char *applicationName;
    char *processName;
    uint64_t messageId;

    PARCBuffer *payload;
};

static void
_parcLogEntry_Destroy(PARCLogEntry **entryPtr)
{
    PARCLogEntry *entry = *entryPtr;

    parcMemory_Deallocate((void **) &entry->hostName);
    parcMemory_Deallocate((void **) &entry->applicationName);
    parcMemory_Deallocate((void **) &entry->processName);
    parcBuffer_Release(&entry->payload);
}

static char *
_toString(const PARCLogEntry *entry)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();

    parcBufferComposer_Format(composer, "%ld.%06d %d ",
                              (long) entry->timeStamp.tv_sec, (int) entry->timeStamp.tv_usec, entry->level);

    size_t position = parcBuffer_Position(entry->payload);
    parcBufferComposer_PutBuffer(composer, entry->payload);
    parcBuffer_SetPosition(entry->payload, position);

    PARCBuffer *buffer = parcBufferComposer_GetBuffer(composer);
    parcBuffer_Rewind(buffer);

    char *result = parcBuffer_ToString(buffer);
    parcBufferComposer_Release(&composer);

    return result;
}

parcObject_ExtendPARCObject(PARCLogEntry, _parcLogEntry_Destroy, NULL, _toString, NULL, NULL, NULL, NULL);

PARCLogEntry *
parcLogEntry_Create(PARCLogLevel level,
                    const char *hostName,
                    const char *applicationName,
                    const char *processName,
                    const uint64_t messageId,
                    const struct timeval timeStamp,
                    PARCBuffer *payload)
{
    PARCLogEntry *result = parcObject_CreateInstance(PARCLogEntry);
    if (result == NULL) {
        trapOutOfMemory("Creating an instance of PARCLogEntry.");
    }
    result->version = _parcLog_Version;
    result->timeStamp = timeStamp;
    result->hostName = parcMemory_StringDuplicate(hostName, strlen(hostName));
    result->applicationName = parcMemory_StringDuplicate(applicationName, strlen(applicationName));
    result->processName = parcMemory_StringDuplicate(processName, strlen(processName));
    result->messageId = messageId;
    result->level = level;
    result->payload = parcBuffer_Acquire(payload);

    return result;
}

parcObject_ImplementAcquire(parcLogEntry, PARCLogEntry);

parcObject_ImplementRelease(parcLogEntry, PARCLogEntry);

PARCBuffer *
parcLogEntry_GetPayload(const PARCLogEntry *instance)
{
    return instance->payload;
}

const struct timeval *
parcLogEntry_GetTimeStamp(const PARCLogEntry *instance)
{
    return &instance->timeStamp;
}

PARCLogLevel
parcLogEntry_GetLevel(const PARCLogEntry *instance)
{
    return instance->level;
}

int
parcLogEntry_GetVersion(const PARCLogEntry *instance)
{
    return instance->version;
}

const char *
parcLogEntry_GetHostName(const PARCLogEntry *instance)
{
    return instance->hostName;
}

const char *
parcLogEntry_GetApplicationName(const PARCLogEntry *instance)
{
    return instance->applicationName;
}

const char *
parcLogEntry_GetProcessName(const PARCLogEntry *instance)
{
    return instance->processName;
}

uint64_t
parcLogEntry_GetMessageId(const PARCLogEntry *instance)
{
    return instance->messageId;
}

char *
parcLogEntry_ToString(const PARCLogEntry *entry)
{
    return _toString(entry);
}
