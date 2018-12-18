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
#include <stdarg.h>

#include <LongBow/longBow_Main.h>

#include <LongBow/unit-test.h>
#include <LongBow/Reporting/longBowReport_Testing.h>
#include <LongBow/longBow_Config.h>

int
longBowMain_Impl(int argc, char *argv[argc], ...)
{
    LongBowStatus exitStatus = LONGBOW_STATUS_SUCCEEDED;

    // Perform some processing on the input parameters.

    LongBowConfig *config = longBowConfig_Create(argc, argv, NULL);
    if (config == NULL) {
        return LONGBOW_STATUS_FAILED;
    }
    va_list ap;
    va_start(ap, argv);

    for (LongBowTestRunner *testRunner = va_arg(ap, LongBowTestRunner *); testRunner != NULL; testRunner = va_arg(ap, LongBowTestRunner *)) {
        if (testRunner != NULL) {
            longBowTestRunner_SetConfiguration(testRunner, config);
            longBowTestRunner_Run(testRunner);
            longBowReportTesting_TestRunner(testRunner);

            if (!longBowTestRunner_IsSuccessful(testRunner)) {
                exitStatus = longBowTestRunner_GetStatus(testRunner);
            }
        }
    }
    va_end(ap);

    longBowConfig_Destroy(&config);

    return (int) exitStatus;
}
