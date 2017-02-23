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

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Buffer.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_Types.h>
#include <ccnx/common/codec/ccnxCodec_TlvUtilities.h>

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_HashCodec.h>

/**
 * These are the accepted sizes for the pre-defined hash types.
 * Hash TLVs with lengths that do not match one of these values will be deemed
 * invalid and not parse correctly.
 */
static const size_t _PARCCryptoHashType_SHA256_Sizes[] = { 32 };
static const size_t _PARCCryptoHashType_SHA512_Sizes[] = { 32, 64 };

static bool
_ccnxCodecSchemaV1HashCodec_ValidHashSize(size_t size, size_t numSizes, size_t sizes[numSizes])
{
    for (size_t i = 0; i < numSizes; i++) {
        if (sizes[i] == size) {
            return true;
        }
    }
    return false;
}

ssize_t
ccnxCodecSchemaV1HashCodec_Encode(CCNxCodecTlvEncoder *encoder, const PARCCryptoHash *hash)
{
    PARCBuffer *digest = parcCryptoHash_GetDigest(hash);
    PARCCryptoHashType hashType = parcCryptoHash_GetDigestType(hash);
    size_t digestLength = parcBuffer_Remaining(digest);

    uint16_t tlvHashType = CCNxCodecSchemaV1Types_HashType_App;
    bool validHash = true;
    switch (hashType) {
        case PARCCryptoHashType_SHA256:
            tlvHashType = CCNxCodecSchemaV1Types_HashType_SHA256;
            validHash = _ccnxCodecSchemaV1HashCodec_ValidHashSize(digestLength,
                                                                  sizeof(_PARCCryptoHashType_SHA256_Sizes) / sizeof(size_t), (size_t *) _PARCCryptoHashType_SHA256_Sizes);
            break;
        case PARCCryptoHashType_SHA512:
            tlvHashType = CCNxCodecSchemaV1Types_HashType_SHA512;
            validHash = _ccnxCodecSchemaV1HashCodec_ValidHashSize(digestLength,
                                                                  sizeof(_PARCCryptoHashType_SHA512_Sizes) / sizeof(size_t), (size_t *) _PARCCryptoHashType_SHA512_Sizes);
            break;
        default:
            break;
    }

    if (validHash) {
        return ccnxCodecTlvEncoder_AppendBuffer(encoder, tlvHashType, digest);
    } else {
        CCNxCodecError *error = ccnxCodecError_Create(TLV_MISSING_MANDATORY, __func__, __LINE__, ccnxCodecTlvEncoder_Position(encoder));
        ccnxCodecTlvEncoder_SetError(encoder, error);
        ccnxCodecError_Release(&error);

        return -1;
    }
}

static bool
_ccnxCodecSchemaV1HashCodec_ValidHash(uint16_t hashType, uint16_t hashSize)
{
    bool validHash = true;

    switch (hashType) {
        case CCNxCodecSchemaV1Types_HashType_SHA256:
            validHash = _ccnxCodecSchemaV1HashCodec_ValidHashSize(hashSize,
                                                                  sizeof(_PARCCryptoHashType_SHA256_Sizes) / sizeof(size_t), (size_t *) _PARCCryptoHashType_SHA256_Sizes);
            break;
        case CCNxCodecSchemaV1Types_HashType_SHA512:
            validHash = _ccnxCodecSchemaV1HashCodec_ValidHashSize(hashSize,
                                                                  sizeof(_PARCCryptoHashType_SHA512_Sizes) / sizeof(size_t), (size_t *) _PARCCryptoHashType_SHA512_Sizes);
            break;
        default:
            break;
    }

    return validHash;
}

PARCCryptoHash *
ccnxCodecSchemaV1HashCodec_DecodeValue(CCNxCodecTlvDecoder *decoder, size_t limit)
{
    PARCCryptoHash *hash = NULL;
    uint16_t hashType = 0;
    uint16_t length = 0;

    if (ccnxCodecTlvDecoder_EnsureRemaining(decoder, 4)) {
        hashType = ccnxCodecTlvDecoder_GetType(decoder);
        length = ccnxCodecTlvDecoder_GetLength(decoder);

        if (length > limit) {
            CCNxCodecError *error = ccnxCodecError_Create(TLV_MISSING_MANDATORY, __func__, __LINE__, ccnxCodecTlvDecoder_Position(decoder));
            ccnxCodecTlvDecoder_SetError(decoder, error);
            ccnxCodecError_Release(&error);
            return NULL;
        }

        if (ccnxCodecTlvDecoder_EnsureRemaining(decoder, length)) {
            PARCBuffer *value = ccnxCodecTlvDecoder_GetValue(decoder, length);
            switch (hashType) {
                case CCNxCodecSchemaV1Types_HashType_SHA256:
                    hash = parcCryptoHash_Create(PARCCryptoHashType_SHA256, value);
                    break;
                case CCNxCodecSchemaV1Types_HashType_SHA512:
                    hash = parcCryptoHash_Create(PARCCryptoHashType_SHA512, value);
                    break;
                case CCNxCodecSchemaV1Types_HashType_App:
                    hash = parcCryptoHash_Create(PARCCryptoHashType_NULL, value);
                    break;
            }
            parcBuffer_Release(&value);
        }
    }

    // Verify the hash size, if one was parsed correctly.
    if (hash != NULL && !_ccnxCodecSchemaV1HashCodec_ValidHash(hashType, length)) {
        parcCryptoHash_Release(&hash);
    }

    return hash;
}
