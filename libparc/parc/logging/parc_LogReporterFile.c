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
#include <inttypes.h>

#include <parc/logging/parc_LogReporterFile.h>
#include <parc/logging/parc_LogFormatSyslog.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Time.h>

#include <parc/algol/parc_FileOutputStream.h>

PARCLogReporter *
parcLogReporterFile_Create(PARCOutputStream *output)
{
    PARCLogReporter *result = parcLogReporter_Create(&parcLogReporterFile_Acquire,
                                                     parcLogReporterFile_Release,
                                                     parcLogReporterFile_Report,
                                                     parcOutputStream_Acquire(output));
    return result;
}

PARCLogReporter *
parcLogReporterFile_Acquire(const PARCLogReporter *reporter)
{
    return parcObject_Acquire(reporter);
}

void
parcLogReporterFile_Release(PARCLogReporter **reporterP)
{
    parcObject_Release((void **) reporterP);
}

void
parcLogReporterFile_Report(PARCLogReporter *reporter, const PARCLogEntry *entry)
{
    PARCOutputStream *output = parcLogReporter_GetPrivateObject(reporter);

    PARCBuffer *formatted = parcLogFormatSyslog_FormatEntry(entry);
    parcOutputStream_Write(output, formatted);
    parcBuffer_Release(&formatted);
}
