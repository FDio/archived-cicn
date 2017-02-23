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

#include "../unit-test.h"

LONGBOW_TEST_RUNNER(Unimplemented)
{
    LONGBOW_RUN_TEST_FIXTURE(Unimplemented);
}

LONGBOW_TEST_RUNNER_SETUP(Unimplemented)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_RUNNER_TEARDOWN(Unimplemented)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Unimplemented)
{
    LONGBOW_RUN_TEST_CASE(Unimplemented, Unimplemented);
}

LONGBOW_TEST_FIXTURE_SETUP(Unimplemented)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Unimplemented)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Unimplemented, Unimplemented)
{
    testUnimplemented("Unimplemented");
}

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(Unimplemented);
    int status = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);

    exit(status);
}
