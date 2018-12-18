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
 * @file longBow_Status.h
 * @ingroup testing
 * @brief A simple status representation for a LongBow Test Case.
 *
 */
#ifndef LongBow_longBow_Status_h
#define LongBow_longBow_Status_h

#include <stdbool.h>
#include <stdint.h>

/**
 * @typedef LongBowStatus
 * @brief The status of an individual Test Case, and aggregate Test Fixture, or Test Runner.
 *
 * Status is either successful or not and each has a subset of qualifiers.
 *
 * A successful status is an outright success, or a qualified success.
 *
 * Qualified success is a test that issued an explicit warning (via <code>testWarn(...)</code>),
 * was purposefully skipped (via <code>testSkip(...)</code>),
 * or is unimplemented (via <code>testUnimplemented(...)</code>).
 *
 * Correspondingly, an unsuccessful test is outright failure,
 * or a qualified failure.
 * Qualified values indicate that the test process received a signal during the test,
 * or either the Test Fixture set-up or tear-down steps signaled failure.
 *
 */
enum LongBowStatus {
    /**
     * Used for expressing the expected status.
     */
    LongBowStatus_DONTCARE        = -2,

    /**
     * The test was not run (initial state).
     */
    LongBowStatus_UNTESTED        = -1,

    /* successful */
    /**
     * The test was successful.
     */
    LONGBOW_STATUS_SUCCEEDED       =  0,
    /**
     * The test was successful, but with a warning.
     */

    LongBowStatus_WARNED          = 10,
    /**
     * The test was successful, but the tear down issued a warning.
     */

    LongBowStatus_TEARDOWN_WARNED = 11,

    /**
     * The test was purposefully skipped by the test implementor.
     */
    LONGBOW_STATUS_SKIPPED         = 21,

    /**
     * The test was incomplete because it signals that it is not implemented.
     */
    LongBowStatus_UNIMPLEMENTED   = 22,

    /**
     * The test ran but evaluated nothing.
     */
    LongBowStatus_IMPOTENT        = 23,

    /**
     * The setup function signals that all of the subordinate test cases must be skipped.
     */
    LONGBOW_STATUS_SETUP_SKIPTESTS = 24,

    /* failures */
    /**
     * The tests failed.
     */
    LONGBOW_STATUS_FAILED          = 1,

    /**
     * The test failed because it was stopped by a signal.
     */
    LongBowStatus_STOPPED         = 3,

    /**
     * The tear down of the test failed.
     *
     * This doesn't imply that the test failed.
     */
    LONGBOW_STATUS_TEARDOWN_FAILED = 4,

    /**
     * The test was incomplete because the setup for the test failed.
     */
    LONGBOW_STATUS_SETUP_FAILED    = 5,

    /**
     * The test was incomplete because a memory leak was detected.
     */
    LONGBOW_STATUS_MEMORYLEAK      = 6,

    /**
     * The test failed due to an uncaught signal.
     */
    LongBowStatus_SIGNALLED       = 100,

    /**
     * The limit of LongBowStatus values
     */
    LongBowStatus_LIMIT           = 200,
};
typedef enum LongBowStatus LongBowStatus;

/**
 * Compose a LongBowStatus from the given signalNumber.
 */
#define LongBowStatus_SIGNAL(signalNumber) (LongBowStatus_SIGNALLED + signalNumber)

/**
 * Generate a human readable, nul-terminated C string representation of the `LongBowStatus` value.
 *
 * @param [in] status A `LongBowStatus` value
 *
 * @return A pointer to an allocated C string that must be freed via stdlib.h free(3).
 *
 * Example:
 * @code
 * {
 *     LongBowStatus status = LONGBOW_STATUS_SUCCEEDED;
 *     printf("Status = %s\n", longBowStatus_ToString(status));
 * }
 * @endcode
 */
char *longBowStatus_ToString(LongBowStatus status);

/**
 * Return `true` if the given status indicates an outright or qualified success.
 *
 * Success, outright or qualified, encompasses `LONGBOW_STATUS_SUCCEEDED`,
 *  longBowStatus_IsWarning(status), or
 * longBowStatus_IsIncomplete(status)
 *
 * @param [in] status A `LongBowStatus` value.
 *
 * @return `true` if the given status indicated an outright or qualified success.
 *
 * Example:
 * @code
 * {
 *     LongBowStatus status = LONGBOW_STATUS_SUCCEEDED;
 *     printf("Is success? = %d\n", longBowStatus_IsSuccessful(status));
 * }
 * @endcode
 */
bool longBowStatus_IsSuccessful(LongBowStatus status);

/**
 * Return <code>true</code> if the given status indicates a failure.
 *
 * @param [in] status A `LongBowStatus` value.
 *
 * @return `true` if the given status indicated a failure.
 *
 * Example:
 * @code
 * {
 *     LongBowStatus status = LONGBOW_STATUS_FAILED;
 *     printf("Is warned? = %d\n", longBowStatus_IsFailed(status));
 * }
 * @endcode
 */
bool longBowStatus_IsFailed(LongBowStatus status);

/**
 * Return <code>true</code> if the given status indicates a warning.
 *
 * @param [in] status A `LongBowStatus` value.
 *
 * @return Return `true` if the given status indicate a warning.
 */
bool longBowStatus_IsWarning(LongBowStatus status);

/**
 * Return <code>true</code> if the given status indicates a test was incomplete.
 *
 * @param [in] status A `LongBowStatus` value.
 *
 * @return `true` if the given status indicated it was incomplete.
 *
 * Example:
 * @code
 * {
 *     LongBowStatus status = LONGBOW_STATUS_SKIPPED;
 *     printf("Is incomplete? = %d\n", longBowStatus_IsIncomplete(status));
 * }
 * @endcode
 */
bool longBowStatus_IsIncomplete(LongBowStatus status);

/**
 * Return <code>true</code> if the given status indicated a test induced a signal.
 *
 * @param [in] status A `LongBowStatus` value.
 *
 * @return `true` if the given status indicated a test induced a signal.
 *
 * Example:
 * @code
 * {
 *     LongBowStatus status = LongBowStatus_SIGNALLED;
 *     printf("Is signalled? = %d\n", longBowStatus_IsSignalled(status));
 * }
 * @endcode
 */
bool longBowStatus_IsSignalled(LongBowStatus status);
#endif // LONGBOWSTATUS_H_
