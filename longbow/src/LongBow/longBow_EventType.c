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
 *
 */
#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <signal.h>

#include <LongBow/longBow_Status.h>
#include <LongBow/longBow_EventType.h>

/** @cond private */
struct longbow_event_type {
    const char *kind;
    const char *name;
    LongBowStatus statusCode;
    bool suppressBacktrace;
    bool suppressAlert;
};
/** @endcond */

#define _eventType(_kind, _name, _code, _suppressBacktrace, _suppressAlert) { \
        .kind = _kind, \
        .name = _name, \
        .statusCode = _code, \
        .suppressBacktrace = _suppressBacktrace, \
        .suppressAlert = _suppressAlert \
}

LongBowEventType LongBowAssertEvent = _eventType("Assert", "Assert", LONGBOW_STATUS_FAILED, false, false);

LongBowEventType LongBowTrapEvent = _eventType("Trap", "Trap", LONGBOW_STATUS_FAILED, false, false);

LongBowEventType LongBowTrapOutOfBounds = _eventType("Trap", "OutOfBounds", LONGBOW_STATUS_FAILED, false, false);

LongBowEventType LongBowTrapIllegalValue = _eventType("Trap", "IllegalValue", LONGBOW_STATUS_FAILED, false, false);

LongBowEventType LongBowTrapInvalidValue = _eventType("Trap", "InvalidValue", LONGBOW_STATUS_FAILED, false, false);

LongBowEventType LongBowTrapUnrecoverableState = _eventType("Trap", "UnrecoverableState", LONGBOW_STATUS_FAILED, false, false);

LongBowEventType LongBowTrapNotImplemented = _eventType("Trap", "Implemented", LONGBOW_STATUS_FAILED, false, false);

LongBowEventType LongBowTrapOutOfMemoryEvent = _eventType("Trap", "Out of Memory", LONGBOW_STATUS_FAILED, false, false);

LongBowEventType LongBowTrapCannotObtainLockEvent = _eventType("Trap", "Cannot obtain lock", LONGBOW_STATUS_FAILED, false, false);

LongBowEventType LongBowTrapUnexpectedStateEvent = _eventType("Trap", "Unexpected State", LONGBOW_STATUS_FAILED, false, false);

LongBowEventType LongBowTestSkippedEvent = _eventType("Test", "Skipped", LONGBOW_STATUS_SKIPPED, true, false);

LongBowEventType LongBowTestUnimplementedEvent = _eventType("Test", "Unimplemented", LongBowStatus_UNIMPLEMENTED, true, true);

LongBowEventType LongBowEventSIGHUP = _eventType("Signal", "SIGHUP", LongBowStatus_SIGNAL(SIGHUP), false, false);

LongBowEventType LongBowEventSIGINT = _eventType("Signal", "SIGINT", LongBowStatus_SIGNAL(SIGINT), false, false);

LongBowEventType LongBowEventSIGQUIT = _eventType("Signal", "SIGQUIT", LongBowStatus_SIGNAL(SIGQUIT), false, false);

LongBowEventType LongBowEventSIGILL = _eventType("Signal", "SIGILL", LongBowStatus_SIGNAL(SIGILL), false, false);

LongBowEventType LongBowEventSIGTRAP = _eventType("Signal", "SIGTRAP", LongBowStatus_SIGNAL(SIGTRAP), false, false);

LongBowEventType LongBowEventSIGABRT = _eventType("Signal", "SIGABRT", LongBowStatus_SIGNAL(SIGABRT), false, false);

#ifdef _DARWIN_C_SOURCE
LongBowEventType LongBowEventSIGEMT = _eventType("Signal", "SIGEMT", LongBowStatus_SIGNAL(SIGEMT), false, false);
#else
LongBowEventType LongBowEventSIGEMT = _eventType("Signal", "SIGBUS", LongBowStatus_SIGNAL(SIGBUS), false, false);
#endif

LongBowEventType LongBowEventSIGFPE = _eventType("Signal", "SIGFPE", LongBowStatus_SIGNAL(SIGFPE), false, false);

LongBowEventType LongBowEventSIGKILL = _eventType("Signal", "SIGKILL", LongBowStatus_SIGNAL(SIGKILL), false, false);

LongBowEventType LongBowEventSIGBUS = _eventType("Signal", "SIGBUS", LongBowStatus_SIGNAL(SIGBUS), false, false);

LongBowEventType LongBowEventSIGSEGV = _eventType("Signal", "SIGSEGV", LongBowStatus_SIGNAL(SIGSEGV), false, false);

LongBowEventType LongBowEventSIGSYS = _eventType("Signal", "SIGSYS", LongBowStatus_SIGNAL(SIGSYS), false, false);

LongBowEventType LongBowEventSIGPIPE = _eventType("Signal", "SIGPIPE", LongBowStatus_SIGNAL(SIGPIPE), false, false);

LongBowEventType LongBowEventSIGALRM = _eventType("Signal", "SIGALRM", LongBowStatus_SIGNAL(SIGALRM), false, false);

LongBowEventType LongBowEventSIGTERM = _eventType("Signal", "SIGTERM", LongBowStatus_SIGNAL(SIGTERM), false, false);

LongBowEventType LongBowEventSIGURG = _eventType("Signal", "SIGURG", LongBowStatus_SIGNAL(SIGURG), false, false);

LongBowEventType LongBowEventSIGSTOP = _eventType("Signal", "SIGSTOP", LongBowStatus_SIGNAL(SIGSTOP), false, false);

LongBowEventType LongBowEventSIGTSTP = _eventType("Signal", "SIGTSTP", LongBowStatus_SIGNAL(SIGTSTP), false, false);

LongBowEventType LongBowEventSIGCONT = _eventType("Signal", "SIGCONT", LongBowStatus_SIGNAL(SIGCONT), false, false);

LongBowEventType LongBowEventSIGCHLD = _eventType("Signal", "SIGCHLD", LongBowStatus_SIGNAL(SIGCHLD), false, false);

LongBowEventType LongBowEventSIGTTIN = _eventType("Signal", "SIGTTIN", LongBowStatus_SIGNAL(SIGTTIN), false, false);

LongBowEventType LongBowEventSIGTTOU = _eventType("Signal", "SIGTTOU", LongBowStatus_SIGNAL(SIGTTOU), false, false);

LongBowEventType LongBowEventSIGIO = _eventType("Signal", "SIGIO", LongBowStatus_SIGNAL(SIGIO), false, false);

LongBowEventType LongBowEventSIGXCPU = _eventType("Signal", "SIGXCPU", LongBowStatus_SIGNAL(SIGXCPU), false, false);

LongBowEventType LongBowEventSIGXFSZ = _eventType("Signal", "SIGXFSZ", LongBowStatus_SIGNAL(SIGXFSZ), false, false);

LongBowEventType LongBowEventSIGVTALRM = _eventType("Signal", "SIGVTALRM", LongBowStatus_SIGNAL(SIGVTALRM), false, false);

LongBowEventType LongBowEventSIGPROF = _eventType("Signal", "SIGPROF", LongBowStatus_SIGNAL(SIGPROF), false, false);

LongBowEventType LongBowEventSIGWINCH = _eventType("Signal", "SIGWINCH", LongBowStatus_SIGNAL(SIGWINCH), false, false);

#if  (!defined (__ANDROID__) && (!defined(_POSIX_C_SOURCE)) || defined(_DARWIN_C_SOURCE))
LongBowEventType LongBowEventSIGINFO = _eventType("Signal", "SIGINFO", LongBowStatus_SIGNAL(SIGINFO), false, false);
#else
LongBowEventType LongBowEventSIGINFO = _eventType("Signal", "SIGIO", LongBowStatus_SIGNAL(SIGIO), false, false);
#endif

LongBowEventType LongBowEventSIGUSR1 = _eventType("Signal", "SIGUSR1", LongBowStatus_SIGNAL(SIGUSR1), false, false);

LongBowEventType LongBowEventSIGUSR2 = _eventType("Signal", "SIGUSR2", LongBowStatus_SIGNAL(SIGUSR2), false, false);

LongBowEventType LongBowTestEvent = _eventType("Test", "Test", LongBowStatus_WARNED, false, false);

static LongBowEventType *signalToEventType[NSIG] = {
    NULL,
    &LongBowEventSIGHUP /* 1 */,
    &LongBowEventSIGINT /* 2 */,
    &LongBowEventSIGQUIT /* 3 */,
    &LongBowEventSIGILL /* 4 */,
    &LongBowEventSIGTRAP /* 5 */,
    &LongBowEventSIGABRT /* 6 */,
    &LongBowEventSIGEMT /* 7 */,
    &LongBowEventSIGFPE /* 8 */,
    &LongBowEventSIGKILL /* 9 */,
    &LongBowEventSIGBUS /* 10 */,
    &LongBowEventSIGSEGV /* 11 */,
    &LongBowEventSIGSYS /* 12 */,
    &LongBowEventSIGPIPE /* 13 */,
    &LongBowEventSIGALRM /* 14 */,
    &LongBowEventSIGTERM /* 15 */,
    &LongBowEventSIGURG /* 16 */,
    &LongBowEventSIGSTOP /* 17 */,
    &LongBowEventSIGTSTP /* 18 */,
    &LongBowEventSIGCONT /* 19 */,
    &LongBowEventSIGCHLD /* 20 */,
    &LongBowEventSIGTTIN /* 21 */,
    &LongBowEventSIGTTOU /* 22 */,
    &LongBowEventSIGIO /* 23 */,
    &LongBowEventSIGXCPU /* 24 */,
    &LongBowEventSIGXFSZ /* 25 */,
    &LongBowEventSIGVTALRM /* 26 */,
    &LongBowEventSIGPROF /* 27 */,
    &LongBowEventSIGWINCH /* 28 */,
    &LongBowEventSIGINFO /* 29 */,
    &LongBowEventSIGUSR1 /* 30 */,
    &LongBowEventSIGUSR2 /* 31 */
};

const char *
longBowEventType_GetName(const LongBowEventType *eventType)
{
    return eventType->name;
}

LongBowStatus
longBowEventType_GetStatus(const LongBowEventType *eventType)
{
    return eventType->statusCode;
}

bool
longBowEventType_IsSuppressBacktrace(const LongBowEventType *eventType)
{
    return eventType->suppressBacktrace;
}

bool
longBowEventType_IsSuppressAlert(const LongBowEventType *eventType)
{
    return eventType->suppressAlert;
}

bool
longBowEventType_Equals(const LongBowEventType *x, const LongBowEventType *y)
{
    if (x == y) {
        return true;
    }
    if (x == NULL || y == NULL) {
        return false;
    }

    if (strcmp(x->name, y->name) == 0) {
        return true;
    }
    return false;
}

LongBowEventType *
longBowEventType_GetEventTypeForSignal(const int signal)
{
    return signalToEventType[signal];
}

