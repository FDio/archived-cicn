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

// Include the file(s) containing the functions to be tested.
// This permits internal static functions to be visible to this Test Framework.
#include "../ccnx_InterestReturnInterface.c"

#include <ccnx/common/ccnx_InterestReturn.h>
#include <ccnx/common/internal/ccnx_InterestDefault.h>

#include <parc/algol/parc_SafeMemory.h>
#include <LongBow/unit-test.h>

#include <stdio.h>

LONGBOW_TEST_RUNNER(ccnx_InterestReturnInterface)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

// The Test Runner calls this function once before any Test Fixtures are run.
LONGBOW_TEST_RUNNER_SETUP(ccnx_InterestReturnInterface)
{
    parcMemory_SetInterface(&PARCSafeMemoryAsPARCMemory);
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(ccnx_InterestReturnInterface)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// ======================================================================================

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, ccnxInterestReturnInterface_GetImplementation);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    uint32_t outstandingAllocations = parcSafeMemory_ReportAllocation(STDERR_FILENO);
    if (outstandingAllocations != 0) {
        printf("%s leaks memory by %d allocations\n", longBowTestCase_GetName(testCase), outstandingAllocations);
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

// ======================================================================================


LONGBOW_TEST_CASE(Global, ccnxInterestReturnInterface_GetImplementation)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/boose/roo/pie");

    CCNxInterest *interestV1 =
        ccnxInterest_CreateWithImpl(&CCNxInterestFacadeV1_Implementation,
                                    name,
                                    CCNxInterestDefault_LifetimeMilliseconds,
                                    NULL,
                                    NULL,
                                    CCNxInterestDefault_HopLimit);
    ccnxName_Release(&name);

    CCNxInterestReturn *interestReturn = ccnxInterestReturn_Create(interestV1, CCNxInterestReturn_ReturnCode_Congestion);

    assertTrue(ccnxInterestReturnInterface_GetInterface(interestReturn) == &CCNxInterestReturnFacadeV1_Implementation,
               "Expected V1 Implementation");

    // Now unset the pointer and see if it gets derived properly.
    ccnxTlvDictionary_SetMessageInterface(interestReturn, NULL);

    assertTrue(ccnxInterestReturnInterface_GetInterface(interestReturn) == &CCNxInterestReturnFacadeV1_Implementation,
               "Expected V1 Implementation");

    ccnxInterestReturn_Release(&interestReturn);
    ccnxInterest_Release(&interestV1);
}


// ======================================================================================

int
main(int argc, char *argv[])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(ccnx_InterestReturnInterface);
    int exitStatus = longBowMain(argc, argv, testRunner, NULL);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
