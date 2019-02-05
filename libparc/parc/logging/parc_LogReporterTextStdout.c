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

#ifndef _WIN32
#include <unistd.h>
#endif

#include <config.h>
#include <inttypes.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_OutputStream.h>
#include <parc/algol/parc_FileOutputStream.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_Time.h>

#include <parc/logging/parc_LogReporterTextStdout.h>
#include <parc/logging/parc_LogFormatText.h>

PARCLogReporter *
parcLogReporterTextStdout_Create(void)
{
    PARCFileOutputStream *fileOutput = parcFileOutputStream_Create(dup(STDOUT_FILENO));
    PARCOutputStream *out = parcFileOutputStream_AsOutputStream(fileOutput);
    parcFileOutputStream_Release(&fileOutput);

    PARCLogReporter *result = parcLogReporter_Create(&parcLogReporterTextStdout_Acquire,
                                                     parcLogReporterTextStdout_Release,
                                                     parcLogReporterTextStdout_Report,
                                                     parcOutputStream_Acquire(out));
    parcOutputStream_Release(&out);
    return result;
}

PARCLogReporter *
parcLogReporterTextStdout_Acquire(const PARCLogReporter *instance)
{
    return parcObject_Acquire(instance);
}

void
parcLogReporterTextStdout_Release(PARCLogReporter **reporterP)
{
    parcObject_Release((void **) reporterP);
}

void
parcLogReporterTextStdout_Report(PARCLogReporter *reporter, const PARCLogEntry *entry)
{
    PARCOutputStream *output = parcLogReporter_GetPrivateObject(reporter);

    PARCBuffer *formatted = parcLogFormatText_FormatEntry(entry);
    parcOutputStream_Write(output, formatted);
    parcBuffer_Release(&formatted);
}
