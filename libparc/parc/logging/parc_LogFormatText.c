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

#include <inttypes.h>

#include <parc/logging/parc_LogFormatText.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_Time.h>

PARCBuffer *
parcLogFormatText_FormatEntry(const PARCLogEntry *entry)
{
    PARCBuffer *payload = parcLogEntry_GetPayload(entry);

    char theTime[64];
    parcTime_TimevalAsRFC3339(parcLogEntry_GetTimeStamp(entry), theTime);

    PARCBufferComposer *composer = parcBufferComposer_Allocate(128);

    parcBufferComposer_PutStrings(composer,
                                  theTime, " ",
                                  parcLogLevel_ToString(parcLogEntry_GetLevel(entry)), " ",
                                  parcLogEntry_GetHostName(entry), " ",
                                  parcLogEntry_GetApplicationName(entry), " ",
                                  parcLogEntry_GetProcessName(entry), " ", NULL);

    parcBufferComposer_Format(composer, "%" PRId64 " [ ", parcLogEntry_GetMessageId(entry));
    parcBufferComposer_PutBuffer(composer, payload);
    parcBufferComposer_PutStrings(composer, " ]\n", NULL);
    PARCBuffer *result = parcBuffer_Flip(parcBuffer_Acquire(parcBufferComposer_GetBuffer(composer)));

    parcBufferComposer_Release(&composer);

    return result;
}
