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

#include <config.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>

#include <parc/logging/parc_Log.h>

#include <ccnx/forwarder/metis/core/metis_Logger.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

struct metis_logger {
    PARCClock *clock;

    PARCLogReporter *reporter;
    PARCLog *loggerArray[MetisLoggerFacility_END];
};

static const struct facility_to_string {
    MetisLoggerFacility facility;
    const char *string;
} _facilityToString[] = {
    { .facility = MetisLoggerFacility_Config,    .string = "Config"    },
    { .facility = MetisLoggerFacility_Core,      .string = "Core"      },
    { .facility = MetisLoggerFacility_IO,        .string = "IO"        },
    { .facility = MetisLoggerFacility_Message,   .string = "Message"   },
    { .facility = MetisLoggerFacility_Processor, .string = "Processor" },
    { .facility = 0,                             .string = NULL        }
};

const char *
metisLogger_FacilityString(MetisLoggerFacility facility)
{
    for (int i = 0; _facilityToString[i].string != NULL; i++) {
        if (_facilityToString[i].facility == facility) {
            return _facilityToString[i].string;
        }
    }
    return "Unknown";
}

static void
_allocateLoggers(MetisLogger *logger, PARCLogReporter *reporter)
{
    trapUnexpectedStateIf(logger->reporter != NULL, "Trying to allocate a reporter when the previous one is not null");
    logger->reporter = parcLogReporter_Acquire(reporter);

    char hostname[255];
    int gotHostName = gethostname(hostname, 255);
    if (gotHostName < 0) {
        snprintf(hostname, 255, "unknown");
    }

    for (int i = 0; i < MetisLoggerFacility_END; i++) {
        logger->loggerArray[i] = parcLog_Create(hostname, metisLogger_FacilityString(i), "metis", logger->reporter);
        parcLog_SetLevel(logger->loggerArray[i], PARCLogLevel_Error);
    }
}

static void
_releaseLoggers(MetisLogger *logger)
{
    for (int i = 0; i < MetisLoggerFacility_END; i++) {
        parcLog_Release(&logger->loggerArray[i]);
    }
    parcLogReporter_Release(&logger->reporter);
}

static void
_destroyer(MetisLogger **loggerPtr)
{
    MetisLogger *logger = *loggerPtr;
    _releaseLoggers(logger);
    parcClock_Release(&(*loggerPtr)->clock);
}

parcObject_ExtendPARCObject(MetisLogger, _destroyer, NULL, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementAcquire(metisLogger, MetisLogger);

parcObject_ImplementRelease(metisLogger, MetisLogger);

MetisLogger *
metisLogger_Create(PARCLogReporter *reporter, const PARCClock *clock)
{
    assertNotNull(reporter, "Parameter reporter must be non-null");
    assertNotNull(clock, "Parameter clock must be non-null");

    MetisLogger *logger = parcObject_CreateAndClearInstance(MetisLogger);
    if (logger) {
        logger->clock = parcClock_Acquire(clock);
        _allocateLoggers(logger, reporter);
    }

    return logger;
}

void
metisLogger_SetReporter(MetisLogger *logger, PARCLogReporter *reporter)
{
    assertNotNull(logger, "Parameter logger must be non-null");

    // save the log level state
    PARCLogLevel savedLevels[MetisLoggerFacility_END];
    for (int i = 0; i < MetisLoggerFacility_END; i++) {
        savedLevels[i] = parcLog_GetLevel(logger->loggerArray[i]);
    }

    _releaseLoggers(logger);

    _allocateLoggers(logger, reporter);

    // restore log level state
    for (int i = 0; i < MetisLoggerFacility_END; i++) {
        parcLog_SetLevel(logger->loggerArray[i], savedLevels[i]);
    }
}

void
metisLogger_SetClock(MetisLogger *logger, PARCClock *clock)
{
    assertNotNull(logger, "Parameter logger must be non-null");
    parcClock_Release(&logger->clock);
    logger->clock = parcClock_Acquire(clock);
}

static void
_assertInvariants(const MetisLogger *logger, MetisLoggerFacility facility)
{
    assertNotNull(logger, "Parameter logger must be non-null");
    trapOutOfBoundsIf(facility >= MetisLoggerFacility_END, "Invalid facility %d", facility);
}

void
metisLogger_SetLogLevel(MetisLogger *logger, MetisLoggerFacility facility, PARCLogLevel minimumLevel)
{
    _assertInvariants(logger, facility);
    PARCLog *log = logger->loggerArray[facility];
    parcLog_SetLevel(log, minimumLevel);
}

bool
metisLogger_IsLoggable(const MetisLogger *logger, MetisLoggerFacility facility, PARCLogLevel level)
{
    _assertInvariants(logger, facility);
    PARCLog *log = logger->loggerArray[facility];
    return parcLog_IsLoggable(log, level);
}

void
metisLogger_Log(MetisLogger *logger, MetisLoggerFacility facility, PARCLogLevel level, const char *module, const char *format, ...)
{
    if (metisLogger_IsLoggable(logger, facility, level)) {
        // this is logged as the messageid
        uint64_t logtime = parcClock_GetTime(logger->clock);

        // metisLogger_IsLoggable asserted invariants so we know facility is in bounds
        PARCLog *log = logger->loggerArray[facility];

        va_list va;
        va_start(va, format);

        parcLog_MessageVaList(log, level, logtime, format, va);

        va_end(va);
    }
}

