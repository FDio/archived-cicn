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

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_CryptoSuite.h>


bool
ccnxCodecSchemaV1CryptoSuite_ParcToTlv(PARCCryptoSuite parcSuite, CCNxCodecSchemaV1TlvDictionary_CryptoSuite *outputValue)
{
    bool matchFound = false;
    switch (parcSuite) {
        case PARCCryptoSuite_RSA_SHA256:
            *outputValue = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_RsaSha256;
            matchFound = true;
            break;

        case PARCCryptoSuite_DSA_SHA256:
            // not supported yet
            break;

        case PARCCryptoSuite_RSA_SHA512:
            // not supported yet
            break;

        case PARCCryptoSuite_HMAC_SHA256:
            *outputValue = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_HmacSha256;
            matchFound = true;
            break;

        case PARCCryptoSuite_HMAC_SHA512:
            // not supported yet
            break;

        case PARCCryptoSuite_NULL_CRC32C:
            *outputValue = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_CRC32C;
            matchFound = true;
            break;

        default:
            // unknown
            break;
    }
    return matchFound;
}

bool
ccnxCodecSchemaV1CryptoSuite_TlvToParc(CCNxCodecSchemaV1TlvDictionary_CryptoSuite tlvValue, PARCCryptoSuite *outputSuite)
{
    bool matchFound = false;
    switch (tlvValue) {
        case CCNxCodecSchemaV1TlvDictionary_CryptoSuite_RsaSha256:
            *outputSuite = PARCCryptoSuite_RSA_SHA256;
            matchFound = true;
            break;

        case CCNxCodecSchemaV1TlvDictionary_CryptoSuite_CRC32C:
            *outputSuite = PARCCryptoSuite_NULL_CRC32C;
            matchFound = true;
            break;

        case CCNxCodecSchemaV1TlvDictionary_CryptoSuite_HmacSha256:
            *outputSuite = PARCCryptoSuite_HMAC_SHA256;
            matchFound = true;
            break;

        case CCNxCodecSchemaV1TlvDictionary_CryptoSuite_EcSecp256K1:
            // not supported yet
            break;

        default:
            // unknown
            break;
    }
    return matchFound;
}

bool
ccnxCodecSchemaV1CryptoSuite_SignAndHashToTlv(PARCSigningAlgorithm signAlgorithm, PARCCryptoHashType hashType, CCNxCodecSchemaV1TlvDictionary_CryptoSuite *outputValue)
{
    bool matchFound = false;
    switch (signAlgorithm) {
        case PARCSigningAlgorithm_RSA: {
            switch (hashType) {
                case PARCCryptoHashType_SHA256:
                    *outputValue = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_RsaSha256;
                    matchFound = true;

                default:
                    break;
            }
            break;
        }

        case PARCSigningAlgorithm_HMAC: {
            switch (hashType) {
                case PARCCryptoHashType_SHA256:
                    *outputValue = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_HmacSha256;
                    matchFound = true;
                default:
                    break;
            }
            break;
        }

        case PARCSigningAlgortihm_NULL: {
            switch (hashType) {
                case PARCCryptoHashType_CRC32C:
                    *outputValue = CCNxCodecSchemaV1TlvDictionary_CryptoSuite_CRC32C;
                    matchFound = true;
                default:
                    break;
            }
            break;
        }
        default:
            break;
    }
    return matchFound;
}

