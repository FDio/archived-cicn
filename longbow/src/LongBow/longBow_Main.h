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
 * @file longBow_Main.h
 * @ingroup testing
 * @brief A main() function to run one or more LongBow Test Runners.
 *
 * The functions in this file provide execution time support for running LongBow Tests.
 *
 */
#ifndef LONGBOWMAIN_H_
#define LONGBOWMAIN_H_

/**
 * Run one or more LongBow Test Runners.
 *
 * The encapsulating function creates one or more `LongBowTestRunner` instances and supplies
 * these as a NULL terminated variable argument list to the longBowMain function.
 * The return value from longBowMain is suitable as an exit status from an executable as zero is success.
 *
 * @param [in] argc  The number of elements in argv.
 * @param [in] argv  An array of C string arguments.
 *
 * @return 0 All tests for all LongBowTestRunners were successful. Otherwise one of `LongBowStatus`.
 *
 * Example Usage:
 * @code
 * int
 * main(int argc, char *argv[argc])
 * {
 *   LongBowTestRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(MyTestRunner);
 *   int exitStatus = longBowMain(argc, argv, testRunner, NULL);
 *   longBowTestRunner_Destroy(&testRunner);
 *   exit(exitStatus);
 * }
 * @endcode
 */
int longBowMain_Impl(int argc, char *argv[], ...);
#endif /* LONGBOWMAIN_H_ */
