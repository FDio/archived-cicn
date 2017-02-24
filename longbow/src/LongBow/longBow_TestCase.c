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

#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>
#include <setjmp.h>
#include <assert.h>
#include <errno.h>

#include <LongBow/unit-test.h>

#include <LongBow/Reporting/longBowReport_Testing.h>

#include <LongBow/longBow_Config.h>
#include <LongBow/longBow_TestCaseClipBoard.h>
#include <LongBow/private/longBow_Memory.h>
#include <LongBow/private/longBow_String.h>


/**
 * @struct longbow_testcase
 * @brief The specification of a LongBow Test Case.
 *
 */
struct longbow_testcase {
    /**
     * The name of the test case as a C string.
     */
    const char *testCaseName;

    const LongBowTestCaseMetaData  *metaData;

    char *fullName;

    /**
     * The fixture that is running this test case.
     */
    const LongBowTestFixture *fixture;

    /**
     * The function to execute the test case.
     */
    LongBowTestCaseFunction *testCase;

    /**
     * The runtime characteristics consisting of the expected and actual results.
     */
    LongBowRuntime *runtime;
};


void
longBowTestCase_ConfigHelp(void)
{
    printf("Test Case options:\n");
    printf("  --set <runnerName>/<fixtureName>/iterations=<integer>  Run the named test case <integer> times.\n");
}

LongBowTestCase *
longBowTestCase_Create(const char *testCaseName,
                       const LongBowTestFixture *testFixture,
                       LongBowTestCaseFunction *testCase,
                       const LongBowTestCaseMetaData  *metaData)
{
    assert(testCaseName != NULL);

    LongBowTestCase *result = longBowMemory_Allocate(sizeof(LongBowTestCase));
    if (result != NULL) {
        result->testCaseName = testCaseName;
        int status = asprintf(&result->fullName, "%s/%s", longBowTestFixture_GetFullName(testFixture), testCaseName);
        assert(status != -1);
        result->fixture = testFixture;
        result->testCase = testCase;
        result->runtime = longBowRuntime_Create(&metaData->expectedResult,
                                                longBowTestRunner_GetConfiguration(longBowTestFixture_GetRunner(testFixture)));
        result->metaData = metaData;
    }
    return result;
}

const char *
longBowTestCase_GetFullName(const LongBowTestCase *testCase)
{
    assert(testCase != NULL);
    return testCase->fullName;
}

void
longBowTestCase_Destroy(LongBowTestCase **testCaseP)
{
    LongBowTestCase *testCase = *testCaseP;

    longBowRuntime_Destroy(&(testCase->runtime));

    longBowMemory_Deallocate((void **) testCaseP);
}

/*
 * Given the exit status of a test as returned by wait(2), return the corresponding {@link LongBowStatus}.
 *
 * @param [in] waitStatus
 * @return LongBowStatus
 */
static LongBowStatus
_longBowTestCase_ParseWaitStatus(int waitStatus)
{
    LongBowStatus result = LongBowStatus_WARNED;
    if (WIFSIGNALED(waitStatus)) {
        int exitSignal = WTERMSIG(waitStatus);
        if (exitSignal == SIGABRT) {
            result = LONGBOW_STATUS_FAILED;
        } else {
            result = LongBowStatus_SIGNALLED + exitSignal;
        }
    } else if (WIFEXITED(waitStatus)) {
        result = (LongBowStatus) WEXITSTATUS(waitStatus);
    } else {
        result = LongBowStatus_STOPPED;
    }

    return result;
}

LongBowLocation *
longBowTestCase_CreateLocation(const LongBowTestCase *testCase)
{
    LongBowLocation *result = longBowLocation_Create(testCase->metaData->fileName, testCase->fullName, testCase->metaData->lineNumber);
    return result;
}

const char *
longBowTestCase_GetName(const LongBowTestCase *testCase)
{
    return testCase->testCaseName;
}

LongBowTestFixture *
longBowTestCase_GetFixture(const LongBowTestCase *testCase)
{
    return (LongBowTestFixture *) testCase->fixture;
}

LongBowRuntimeResult *
longBowTestCase_GetExpectedResult(const LongBowTestCase *testCase)
{
    return longBowRuntime_GetExpectedTestCaseResult(testCase->runtime);
}

LongBowRuntimeResult *
longBowTestCase_GetActualResult(const LongBowTestCase *testCase)
{
    return longBowRuntime_GetActualTestCaseResult(testCase->runtime);
}

size_t
longBowTestCase_GetEventEvaluationCount(const LongBowTestCase *testCase)
{
    return longBowRuntime_GetActualEventEvaluationCount(testCase->runtime);
}

static jmp_buf longBowTestCaseAbort;

/*
 * The process running the Test Case receives a signal.
 *
 *     A regular, passing Test Case induces no signal and as a result does not pass through this function.
 *     A Test Case that fails an assertion will induce the SIGABRT signal which does pass through this function.
 *     Any other signal is because the Test Case either purposefully sent itself a signal, (including calling abort()),
 *     or it induced one through some implicit behaviour (like a SIGSEGV).
 *
 *     In all remaining cases, encode the signal received into a return value for the
 *     corresponding setjmp() and invoke the longjmp().
 */
__attribute__((noreturn))
static void
_longBowTestCase_ReceiveSignal(int signal, siginfo_t *siginfo __attribute__((unused)), void *data __attribute__((unused)))
{
    if (signal == SIGINT) {
        printf("Howdy\n");
    }
    longjmp(longBowTestCaseAbort, signal);
}

/**
 * Return true or false, if LongBow should capture a signal, or not.
 *
 * LongBow captures signals in order to report on the success or failure of a test.
 * Some signals do not indicate that a test failed,
 * only that the environment changed,
 * or some other event that is unrelated to success or failure.
 * Unless, of course, the test is checking for the event (this is not yet supported).
 */
static bool
_longBowTestCase_MustCaptureSignal(const int signal)
{
    switch (signal) {
        case SIGTRAP:
            return false;

        case SIGCHLD:
            return false;

        case SIGKILL:
            return false;

        case SIGSTOP:
            return false;

        case SIGWINCH:
            return false;

        default:
            return true;
    }
}

/**
 * Setup signals to point to the testCaseSignalAction handler for all signals possible.
 */
static void
_longBowTestCase_TestInitSignals(void)
{
    struct sigaction signalAction;
    signalAction.sa_sigaction = _longBowTestCase_ReceiveSignal;
    signalAction.sa_flags = SA_SIGINFO;
    sigemptyset(&signalAction.sa_mask);

    struct sigaction oldSignals[NSIG];
    for (int i = 1; i < NSIG; i++) {
        if (_longBowTestCase_MustCaptureSignal(i)) {
            sigaction(i, &signalAction, &oldSignals[i]);
        }
    }
}

/**
 * Setup signals to their default behaviour.
 */
static void
_longBowTestCase_TestFiniSignals(void)
{
    struct sigaction signalAction;
    signalAction.sa_handler = SIG_DFL;
    signalAction.sa_flags = SA_SIGINFO;
    sigemptyset(&signalAction.sa_mask);

    for (int i = 1; i < NSIG; i++) {
        if (_longBowTestCase_MustCaptureSignal(i)) {
            sigaction(i, &signalAction, NULL);
        }
    }
}

/*
 * Determine the status of the given LongBowTestCase.
 *
 * If the actual event recorded in the test case is equal to the expected event,
 * then return LONGBOW_STATUS_SUCCEEDED.
 * If the actual event is NULL and the expected event is not, then return LONGBOW_STATUS_FAILED.
 * Otherwise, the result is the actual event.
 */
static LongBowStatus
_longBowTestCase_ExpectedVsActualEvent(const LongBowTestCase *testCase)
{
    LongBowStatus result = LONGBOW_STATUS_FAILED;
    LongBowEventType *expectedEvent = longBowRuntimeResult_GetEvent(longBowTestCase_GetExpectedResult(testCase));
    LongBowEventType *actualEvent = longBowRuntimeResult_GetEvent(longBowTestCase_GetActualResult(testCase));

    if (longBowEventType_Equals(expectedEvent, actualEvent)) {
        result = LONGBOW_STATUS_SUCCEEDED;
    } else if (actualEvent == NULL && expectedEvent != NULL) {
        LongBowString *messageString = longBowString_CreateFormat("failed to induce an expected %s event.",
                                                                  longBowEventType_GetName(expectedEvent));

        LongBowLocation *location = longBowTestCase_CreateLocation(testCase);
        LongBowEvent *event = longBowEvent_Create(expectedEvent, location, "", longBowString_ToString(messageString), NULL);

        longBowReportRuntime_Event(event);
        longBowEvent_Destroy(&event);
        longBowString_Destroy(&messageString);

        result = LONGBOW_STATUS_FAILED;
    } else {
        result = longBowEventType_GetStatus(actualEvent);
    }

    return result;
}

/*
 * Invoke the test case function and see if it succeeds.
 *
 * The technique here is to assume the test case will succeed,
 * setup a longjmp back to this function
 * and then setup signal handling and invoke the test case.
 * This essentially wraps the test function an catches and handles the abort()
 * (SIGABRT) that results from the LongBow assertion or trap.
 *
 * If the test case simply returns (ie. the longjmp was never executed),
 * then it was successful.
 * Otherwise, the longjmp contains a code indicating the status captured by
 * the signal handler that was invoked when the test code executed the abort() function.
 * Extract relevant information from the current runtime context
 * (which is always present whether in a unit test case or not)
 *
 * Note that it's actually not necessary for the test code to execute an abort(),
 * it could invoke the longjmp() directly.
 * Something that might be advantagous in the future.
 */
static LongBowStatus
_longBowTestCase_Execute(LongBowTestCase *testCase)
{
    LongBowStatus result = LONGBOW_STATUS_SUCCEEDED;

    if (longBowConfig_IsTrace(longBowTestCase_GetConfiguration(testCase))) {
        longBowReportTesting_Trace("        %s/%s/%s",
                                   longBowTestRunner_GetName(longBowTestFixture_GetRunner(longBowTestCase_GetFixture(testCase))),
                                   longBowTestFixture_GetName(longBowTestCase_GetFixture(testCase)),
                                   longBowTestCase_GetName(testCase));
    }

    int receivedSignal = setjmp(longBowTestCaseAbort);
    if (receivedSignal == 0) {
        _longBowTestCase_TestInitSignals();

        errno = 0;

        LongBowClipBoard *testClipBoard = longBowTestFixture_GetClipBoard(longBowTestCase_GetFixture(testCase));

        (testCase->testCase)(longBowTestFixture_GetRunner(longBowTestCase_GetFixture(testCase)),
                             longBowTestCase_GetFixture(testCase),
                             testCase,
                             testClipBoard,
                             longBowTestCaseAbort);
    } else {
        // We get here as the result of an extraordinary abort from the testCase invoked just above.
        // Sort out the meaning of the received signal here.
        //
        // If an SIGABRT is received, we expect that the Global Runtime contains a valid LongBowEventType.
        // If not, then an old-timey <assert.h> assert() was used and the programmer should be warned,
        // or the system under test actually uses SIGABRT and the programmer is warned that this
        // incompatible with LongBow.

        LongBowEventType *event = NULL;
        if (receivedSignal == SIGABRT) {
            if (longBowRuntime_GetActualEventType(longBowRuntime_GetCurrentRuntime()) == NULL) {
                char *testCaseString = longBowTestCase_ToString(testCase);
                longBowReportRuntime_Warning("Warning: %s commingling LongBow with assert(3) or with SIGABRT will not work.\n", testCaseString);
                free(testCaseString);
            }

            event = longBowRuntime_GetActualEventType(longBowRuntime_GetCurrentRuntime());
        } else if (receivedSignal == SIGTERM) {
            char *testCaseString = longBowTestCase_ToString(testCase);
            longBowReportRuntime_Warning("\nWarning: %s premature termination.\n", testCaseString);
            free(testCaseString);
            event = longBowEventType_GetEventTypeForSignal(receivedSignal);
        } else if (receivedSignal == SIGINT) {
            char *testCaseString = longBowTestCase_ToString(testCase);
            longBowReportRuntime_Warning("\nWarning: %s interrupted.\n", testCaseString);
            free(testCaseString);
            event = longBowEventType_GetEventTypeForSignal(receivedSignal);
        } else {
            event = longBowEventType_GetEventTypeForSignal(receivedSignal);
        }

        longBowRuntimeResult_SetEvent(longBowTestCase_GetActualResult(testCase), event);
    }

    result = _longBowTestCase_ExpectedVsActualEvent(testCase);

    if (result == LONGBOW_STATUS_SUCCEEDED) {
        if (longBowTestCase_GetEventEvaluationCount(testCase) == 0) {
            result = LongBowStatus_IMPOTENT;
        }
    } else if (longBowStatus_IsFailed(result)) {
        ;
    }

    memset(&longBowTestCaseAbort, 0, sizeof(longBowTestCaseAbort));

    _longBowTestCase_TestFiniSignals();

    return result;
}

static LongBowStatus
_longBowTestCase_ExecuteOnce(LongBowTestCase *testCase)
{
    LongBowStatus result = LONGBOW_STATUS_FAILED;

    LongBowTestFixture *fixture = longBowTestCase_GetFixture(testCase);

    LongBowStatus setupStatus = longBowTestFixture_Setup(fixture, testCase);

    if (longBowStatus_IsSuccessful(setupStatus) == true) {
        LongBowStatus testCaseStatus = _longBowTestCase_Execute(testCase);

        LongBowStatus tearDownStatus = longBowTestFixture_TearDown(fixture, testCase);

        // Ensure that things only go from "bad" to "worse." If a test case is indicating a failure
        // and yet the tear-down is also indicating something NOT successful (like a warning), don't
        // demote the status from LONGBOW_STATUS_FAILED to LONGBOW_STATUS_TEARDOWN_WARNING.
        // This is messy.  Perhaps a better approach would be to structure status as an ordered arrangement of status.

        result = testCaseStatus;

        if (testCaseStatus == LONGBOW_STATUS_SUCCEEDED) {
            if (tearDownStatus != LONGBOW_STATUS_SUCCEEDED) {
                result = tearDownStatus;
            }
        }
    }

    return result;
}

static LongBowStatus
_longBowTestCase_Iterate(LongBowTestCase *testCase)
{
    LongBowStatus result = LONGBOW_STATUS_SUCCEEDED;

    LongBowConfig *config = longBowTestCase_GetConfiguration(testCase);

    uint32_t iterations = longBowConfig_GetUint32(config, 1, "%s/iterations", longBowTestCase_GetFullName(testCase));

    for (uint32_t i = 0; i < iterations && longBowStatus_IsSuccessful(result); i++) {
        LongBowRuntime *previousRuntime = longBowRuntime_SetCurrentRuntime(testCase->runtime);
        result = _longBowTestCase_ExecuteOnce(testCase);
        longBowRuntime_SetCurrentRuntime(previousRuntime);
    }

    return result;
}

/*
 * Run a LongBowTestCase in a forked process.
 *
 * @param testCase The LongBowTestCase to run.
 * @return 0
 */
static int
_longBowTestCase_RunForked(LongBowTestCase *testCase)
{
    struct timeval startTime;
    gettimeofday(&startTime, NULL);

    pid_t pid = fork();
    if (pid == 0) {
        exit(_longBowTestCase_Iterate(testCase));
    } else {
        // Rummage around in various things to obtain the post-mortem
        // results of the test that was run in a separate process.
        int waitStatus;
        struct rusage rusage;
#ifndef _ANDROID_
        wait3(&waitStatus, 0, &rusage);
#else
        wait4(-1, &waitStatus, 0, &rusage);
#endif
        struct timeval endTime;
        gettimeofday(&endTime, NULL);

        struct timeval elapsedTime;
        timersub(&endTime, &startTime, &elapsedTime);

        longBowRuntimeResult_SetElapsedTime(longBowTestCase_GetActualResult(testCase), &elapsedTime);
        longBowRuntimeResult_SetRUsage(longBowTestCase_GetActualResult(testCase), &rusage);
        longBowRuntimeResult_SetStatus(longBowTestCase_GetActualResult(testCase), _longBowTestCase_ParseWaitStatus(waitStatus));
    }
    return 0;
}

/*
 * Run a LongBowTestCase in this address space (ie. not a forked process).
 *
 * @param testCase The LongBowTestCase to run.
 * @return 0
 */
static int
_longBowTestCase_RunNonForked(LongBowTestCase *testCase)
{
    struct timeval startTime;
    gettimeofday(&startTime, NULL);

    LongBowStatus status = _longBowTestCase_Iterate(testCase);

    struct timeval endTime;
    gettimeofday(&endTime, NULL);

    struct timeval elapsedTime;
    timersub(&endTime, &startTime, &elapsedTime);

    longBowRuntimeResult_SetElapsedTime(longBowTestCase_GetActualResult(testCase), &elapsedTime);
    longBowRuntimeResult_SetStatus(longBowTestCase_GetActualResult(testCase), status);

    return 0;
}

LongBowStatus
longBowTestCase_GetExpectedStatus(const LongBowTestCase *testCase)
{
    return longBowRuntimeResult_GetStatus(longBowTestCase_GetExpectedResult(testCase));
}

LongBowTestCase *
longBowTestCase_Run(const char *testCaseName,
                    const LongBowTestFixture *fixture,
                    LongBowTestCaseFunction *testCase,
                    const LongBowTestCaseMetaData  *testCaseMetaData)
{
    LongBowTestCase *result = longBowTestCase_Create(testCaseName, fixture, testCase, testCaseMetaData);

    if (result != NULL) {
        if (longBowConfig_IsRunForked(longBowTestRunner_GetConfiguration(longBowTestFixture_GetRunner(fixture)))) {
            _longBowTestCase_RunForked(result);
        } else {
            _longBowTestCase_RunNonForked(result);
        }

        longBowTestFixture_AddTestCase(fixture, result);
        longBowReportTesting_DisplayTestCaseResult(result);
    }
    return result;
}

LongBowStatus
longBowTestCase_GetStatus(const LongBowTestCase *testCase)
{
    return longBowRuntimeResult_GetStatus(longBowTestCase_GetActualResult(testCase));
}

static LongBowClipBoard *
_longBowTestCase_GetClipBoard(const LongBowTestCase *testCase)
{
    return longBowTestFixture_GetClipBoard(testCase->fixture);
}

void *
longBowTestCase_SetClipBoardData(const LongBowTestCase *testCase, void *data)
{
    return longBowClipBoard_Set(_longBowTestCase_GetClipBoard(testCase), "testCase", data);
}

void *
longBowTestCase_GetClipBoardData(const LongBowTestCase *testCase)
{
    return longBowClipBoard_Get(_longBowTestCase_GetClipBoard(testCase), "testCase");
}

void *
longBowTestCase_Set(const LongBowTestCase *testCase, const char *name, void *value)
{
    return longBowClipBoard_Set(_longBowTestCase_GetClipBoard(testCase), name, value);
}

void *
longBowTestCase_Get(const LongBowTestCase *testCase, const char *name)
{
    return longBowClipBoard_Get(_longBowTestCase_GetClipBoard(testCase), name);
}

char *
longBowClipBoard_GetCString(const LongBowTestCase *testCase, const char *name)
{
    return longBowClipBoard_GetAsCString(_longBowTestCase_GetClipBoard(testCase), name);
}

void *
longBowTestCase_SetInt(const LongBowTestCase *testCase, const char *name, int value)
{
    return longBowClipBoard_SetInt(_longBowTestCase_GetClipBoard(testCase), name, (uint64_t) value);
}

void *
longBowTestCase_SetCString(const LongBowTestCase *testCase, const char *name, char *value)
{
    return longBowClipBoard_SetCString(_longBowTestCase_GetClipBoard(testCase), name, value);
}

int
longBowTestCase_GetInt(const LongBowTestCase *testCase, const char *name)
{
    return (intptr_t) longBowTestCase_Get(testCase, name);
}

LongBowConfig *
longBowTestCase_GetConfiguration(const LongBowTestCase *testCase)
{
    return longBowTestRunner_GetConfiguration(longBowTestFixture_GetRunner(longBowTestCase_GetFixture(testCase)));
}

char *
longBowTestCase_ToString(const LongBowTestCase *testCase)
{
    return strdup(longBowTestCase_GetFullName(testCase));
}

bool
longBowTestCase_IsSuccessful(const LongBowTestCase *testCase)
{
    return longBowStatus_IsSuccessful(longBowTestCase_GetStatus(testCase));
}

bool
longBowTestCase_IsFailed(const LongBowTestCase *testCase)
{
    return longBowStatus_IsFailed(longBowTestCase_GetStatus(testCase));
}

bool
longBowTestCase_IsWarning(const LongBowTestCase *testCase)
{
    return longBowStatus_IsWarning(longBowTestCase_GetStatus(testCase));
}

bool
longBowTestCase_IsIncomplete(const LongBowTestCase *testCase)
{
    return longBowStatus_IsIncomplete(longBowTestCase_GetStatus(testCase));
}
