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

#include <LongBow/runtime.h>

#include <ccnx/common/ccnx_InterestReturn.h>
#include <ccnx/common/ccnx_Interest.h>
#include <ccnx/common/internal/ccnx_InterestReturnFacadeV1.h>

#include <parc/algol/parc_Memory.h>

#include <parc/algol/parc_Object.h>

static const CCNxInterestReturnInterface *_defaultImpl = &CCNxInterestReturnFacadeV1_Implementation;

CCNxInterestReturn *
ccnxInterestReturn_CreateWithImpl(const CCNxInterestReturnInterface *impl,
                                  const CCNxInterest *interest,
                                  CCNxInterestReturn_ReturnCode returnCode)
{
    //The impl must do an Acquire

    CCNxInterestReturn *result = impl->Create(interest, returnCode);

    // And set the dictionary's interface pointer to the one we just used to create this.
    ccnxTlvDictionary_SetMessageInterface(result, impl);

    return result;
}


// Canonical Functions
CCNxInterestReturn *
ccnxInterestReturn_Create(const CCNxInterest *interest, CCNxInterestReturn_ReturnCode returnCode)
{
    CCNxInterestReturn *result =
        ccnxInterestReturn_CreateWithImpl(_defaultImpl, interest, returnCode);
    return result;
}

void
ccnxInterestReturn_AssertValid(const CCNxInterestReturn *interestReturn)
{
    assertNotNull(interestReturn, "Must be a non-null pointer to a CCNxInterestReturn.");
    // Check for required fields in the underlying dictionary. Case 1036
    CCNxInterestReturnInterface *impl = ccnxInterestReturnInterface_GetInterface(interestReturn);
    assertNotNull(impl, "Interest must have an valid implementation pointer.");

    if (impl->AssertValid) {
        impl->AssertValid(interestReturn);
    } else {
        trapNotImplemented("ccnxInterest_GetName");
    }
}

CCNxInterestReturn *
ccnxInterestReturn_Acquire(const CCNxInterestReturn *instance)
{
    return ccnxTlvDictionary_Acquire(instance);
}

void
ccnxInterestReturn_Release(CCNxInterestReturn **instance)
{
    ccnxTlvDictionary_Release(instance);
}

bool
ccnxInterestReturn_Equals(const CCNxInterestReturn *a, const CCNxInterestReturn *b)
{
    if (a == NULL || b == NULL) {
        return false;
    }

    if (a == b) {
        return true;
    }

    CCNxInterestReturnInterface *impl = ccnxInterestReturnInterface_GetInterface(a);
    CCNxInterestReturnInterface *implB = ccnxInterestReturnInterface_GetInterface(b);

    if (impl != implB) {
        return false;
    }

    if (impl->GetReturnCode(a) != impl->GetReturnCode(b)) {
        return false;
    }

    return ccnxInterest_Equals(a, b);
}

char *
ccnxInterestReturn_ToString(const CCNxInterestReturn *interestReturn)
{
    ccnxInterestReturn_AssertValid(interestReturn);

    char *name = ccnxName_ToString(ccnxInterest_GetName(interestReturn));

    CCNxInterestReturnInterface *impl = ccnxInterestReturnInterface_GetInterface(interestReturn);
    CCNxInterestReturn_ReturnCode code = impl->GetReturnCode(interestReturn);

    char *string;
    int len = asprintf(&string, "CCNxInterestReturn{.code=%dms .name=\"%s\"}", code, name);
    if (len < 0) {
        //We have serious problems.
        return name;
    }

    parcMemory_Deallocate((void **) &name);

    char *result = parcMemory_StringDuplicate(string, strlen(string));
    free(string);

    return result;
}

// Accessors Functions
CCNxInterestReturn_ReturnCode
ccnxInterestReturn_GetReturnCode(const CCNxInterestReturn *interestReturn)
{
    ccnxInterestReturn_AssertValid(interestReturn);
    CCNxInterestReturnInterface *impl = ccnxInterestReturnInterface_GetInterface(interestReturn);
    CCNxInterestReturn_ReturnCode code = impl->GetReturnCode(interestReturn);
    return code;
}

