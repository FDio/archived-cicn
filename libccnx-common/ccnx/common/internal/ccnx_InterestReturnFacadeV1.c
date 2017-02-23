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
#include <stdlib.h>
#include <LongBow/runtime.h>

#include <ccnx/common/internal/ccnx_InterestReturnFacadeV1.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>

// =====================

static void
_assertInvariants(const CCNxTlvDictionary *interestDictionary)
{
    assertNotNull(interestDictionary, "Dictionary is null");
    assertTrue(ccnxTlvDictionary_IsInterestReturn(interestDictionary), "Dictionary is not an interest");
    assertTrue(ccnxTlvDictionary_GetSchemaVersion(interestDictionary) == CCNxTlvDictionary_SchemaVersion_V1,
               "Dictionary is wrong schema InterestReturn, got %d expected %d",
               ccnxTlvDictionary_GetSchemaVersion(interestDictionary), CCNxTlvDictionary_SchemaVersion_V1);
}

static uint32_t
_fetchUint32(const CCNxTlvDictionary *interestDictionary, uint32_t key, uint32_t defaultValue)
{
    if (ccnxTlvDictionary_IsValueInteger(interestDictionary, key)) {
        return (uint32_t) ccnxTlvDictionary_GetInteger(interestDictionary, key);
    }
    return defaultValue;
}

// =====================
// Creation

static CCNxTlvDictionary *
_ccnxInterestReturnFacadeV1_Create(
    const CCNxInterest *interest,
    CCNxInterestReturn_ReturnCode code)
{
    assertNotNull(interest, "Parameter name must be non-null");

    assertTrue(ccnxInterestInterface_GetInterface(interest) == &CCNxInterestFacadeV1_Implementation,
               "Non-V1 CCNxInterest passed to V1 ccnxInterestReturn_Create()");

    CCNxInterestReturnFacadeV1_Implementation.interestImpl = CCNxInterestFacadeV1_Implementation;


    CCNxTlvDictionary *dictionary = ccnxTlvDictionary_ShallowCopy(interest);

    //Add InterestReturn specific stuff
    ccnxTlvDictionary_SetMessageType_InterestReturn(dictionary,
                                                    CCNxTlvDictionary_SchemaVersion_V1);

    ccnxTlvDictionary_PutInteger(dictionary,
                                 CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestReturnCode,
                                 code);

    return dictionary;
}

static void
_ccnxInterestReturnFacadeV1_AssertValid(const CCNxTlvDictionary *interestDictionary)
{
    _assertInvariants(interestDictionary);
    assertTrue(ccnxTlvDictionary_IsValueName(interestDictionary, CCNxCodecSchemaV1TlvDictionary_MessageFastArray_NAME), "Name field is not a name");
}

static CCNxInterestReturn_ReturnCode
_ccnxInterestReturnFacadeV1_GetReturnCode(const CCNxTlvDictionary *interestDictionary)
{
    _assertInvariants(interestDictionary);
    CCNxInterestReturn_ReturnCode code = _fetchUint32(interestDictionary,
                                                      CCNxCodecSchemaV1TlvDictionary_HeadersFastArray_InterestReturnCode,
                                                      0);
    assertTrue(((code > 0) && (code < CCNxInterestReturn_ReturnCode_END)), "InterestReturn ReturnCode is out of ranage");

    return code;
}

CCNxInterestReturnInterface CCNxInterestReturnFacadeV1_Implementation = {
    .Create        = &_ccnxInterestReturnFacadeV1_Create,
    .AssertValid   = &_ccnxInterestReturnFacadeV1_AssertValid,
    .GetReturnCode = &_ccnxInterestReturnFacadeV1_GetReturnCode,
};
