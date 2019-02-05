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

#include <parc/logging/parc_LogReporter.h>
#include <parc/algol/parc_Object.h>

struct PARCLogReporter {
    PARCLogReporter *(*acquire)(const PARCLogReporter *);
    void (*release)(PARCLogReporter **);
    void (*report)(PARCLogReporter *, const PARCLogEntry *);

    PARCObject *privateObject;
};

static void
_parcLogReporter_Destroy(PARCLogReporter **reporterPtr __attribute__((unused)))
{
    PARCLogReporter *result = *reporterPtr;
    if (result->privateObject != NULL) {
        parcObject_Release(&result->privateObject);
    }
}

parcObject_ExtendPARCObject(PARCLogReporter, _parcLogReporter_Destroy, NULL, NULL, NULL, NULL, NULL, NULL);

PARCLogReporter *
parcLogReporter_Create(PARCLogReporter *(*acquire)(const PARCLogReporter *),
                       void (*release)(PARCLogReporter **),
                       void (*report)(PARCLogReporter *, const PARCLogEntry *),
                       void *privateObject)
{
    PARCLogReporter *result = parcObject_CreateInstance(PARCLogReporter);
    result->acquire = acquire;
    result->release = release;
    result->report = report;
    result->privateObject = privateObject;

    return result;
}

parcObject_ImplementAcquire(parcLogReporter, PARCLogReporter);

parcObject_ImplementRelease(parcLogReporter, PARCLogReporter);

void
parcLogReporter_Report(PARCLogReporter *reporter, const PARCLogEntry *report)
{
    reporter->report(reporter, report);
}

void *
parcLogReporter_GetPrivateObject(const PARCLogReporter *reporter)
{
    return reporter->privateObject;
}
