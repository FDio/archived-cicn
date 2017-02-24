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
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */

#ifndef CCNx_Common_ccnx_internal_InterestReturnInterface_h
#define CCNx_Common_ccnx_internal_InterestReturnInterface_h

#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <ccnx/common/ccnx_InterestReturn.h>
#include <ccnx/common/ccnx_Interest.h>


typedef struct ccnx_interest_return_interface {
    CCNxInterestInterface interestImpl;

    CCNxTlvDictionary  *(*Create)(const CCNxInterest * interest, CCNxInterestReturn_ReturnCode code);

    bool (*Equals)(const CCNxTlvDictionary *objectA, const CCNxTlvDictionary *objectB);
    void (*AssertValid)(const CCNxTlvDictionary *dict);
    char               *(*ToString)(const CCNxTlvDictionary * dict);

    uint32_t (*GetReturnCode)(const CCNxTlvDictionary *dict);
} CCNxInterestReturnInterface;

extern CCNxInterestReturnInterface CCNxInterestReturnFacadeV1_Implementation;

/**
 * Given a CCNxTlvDictionary representing a CCNxInterestReturn, return the address of the CCNxInterestReturnInterface
 * instance that should be used to access the InterestReturn.
 *
 * @param interestDictionary - a {@link CCNxTlvDictionary} representing a CCNxInterestReturn.
 * @return the address of the `CCNxInterestReturnInterface` instance that should be used to access the CCNxInterestReturn.
 *
 * Example:
 * @code
 * {
 *
 *     CCNxInterest *interest = ...;
 *     CCNxInterestReturn *interestReturn =
 *          ccnxInterestReturn_Create(interest, CCNxInterestReturn_ReturnCode_NoRoute);
 *
 *     //V1 test
 *     if (ccnxInterestReturnInterface_GetInterface(interestReturn) == &CCNxInterestReturnFacadeV1_Implementation) {
 *         printf("Using a V1 CCNxInterestReturnInterface \n");
 *     }
 *
 *     ...
 *
 *     ccnxInterestReturn_Release(&interestReturn);
 * } * @endcode
 */
CCNxInterestReturnInterface *ccnxInterestReturnInterface_GetInterface(const CCNxTlvDictionary *interestDictionary);
#endif
