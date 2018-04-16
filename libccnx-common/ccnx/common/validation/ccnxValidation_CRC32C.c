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
 * See SCTP for a discussion of CRC32C http://tools.ietf.org/html/rfc4960#appendix-B
 * It is also used by iSCSI and other protocols.
 *
 * CRC-32C uses an initial value of 0xFFFFFFFF and a final XOR value of 0xFFFFFFFF.
 *
 */
#include <config.h>
#include <stdio.h>
#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <parc/security/parc_CryptoHasher.h>

#include <ccnx/common/internal/ccnx_ValidationFacadeV1.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>

#include <fcntl.h>
#include <errno.h>

typedef struct crc32_signer {
    PARCCryptoHasher *hasher;
} _CRC32Signer;

typedef struct crc32_verifier {
    PARCCryptoHasher *hasher;
} _CRC32Verifier;

bool
ccnxValidationCRC32C_Set(CCNxTlvDictionary *message)
{
    bool success = true;
    switch (ccnxTlvDictionary_GetSchemaVersion(message)) {
        case CCNxTlvDictionary_SchemaVersion_V1: {
            success &= ccnxTlvDictionary_PutInteger(message, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE, PARCCryptoSuite_NULL_CRC32C);

            break;
        }

        default:
            trapIllegalValue(message, "Unknown schema version: %d", ccnxTlvDictionary_GetSchemaVersion(message));
    }
    return success;
}

bool
ccnxValidationCRC32C_Test(const CCNxTlvDictionary *message)
{
    switch (ccnxTlvDictionary_GetSchemaVersion(message)) {
        case CCNxTlvDictionary_SchemaVersion_V1: {
            if (ccnxTlvDictionary_IsValueInteger(message, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE)) {
                uint64_t cryptosuite = ccnxTlvDictionary_GetInteger(message, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE);
                return (cryptosuite == PARCCryptoSuite_NULL_CRC32C);
            }
            return false;
        }

        default:
            trapIllegalValue(message, "Unknown schema version: %d", ccnxTlvDictionary_GetSchemaVersion(message));
    }
    return false;
}

static bool
_crc32cSigner_Destructor(_CRC32Signer **interfaceContextPtr)
{
    _CRC32Signer *signer = *interfaceContextPtr;
    parcCryptoHasher_Release(&signer->hasher);
    return true;
}

parcObject_ImplementAcquire(_crc32CSigner, _CRC32Signer);
parcObject_ImplementRelease(_crc32CSigner, _CRC32Signer);

parcObject_Override(_CRC32Signer, PARCObject,
                    .destructor = (PARCObjectDestructor *) _crc32cSigner_Destructor);

static bool
_crc32Verifier_Destructor(_CRC32Verifier **verifierPtr)
{
    _CRC32Verifier *verifier = (_CRC32Verifier *) *verifierPtr;

    parcCryptoHasher_Release(&(verifier->hasher));
    return true;
}

parcObject_ImplementAcquire(_crc32Verifier, _CRC32Verifier);
parcObject_ImplementRelease(_crc32Verifier, _CRC32Verifier);

parcObject_Override(_CRC32Verifier, PARCObject,
                    .destructor = (PARCObjectDestructor *) _crc32Verifier_Destructor);

static PARCSignature *
_crc32Signer_SignDigest(_CRC32Signer *interfaceContext, const PARCCryptoHash *cryptoHash)
{
    PARCSignature *signature =
        parcSignature_Create(PARCSigningAlgorithm_NULL, PARCCryptoHashType_CRC32C, parcCryptoHash_GetDigest(cryptoHash));
    return signature;
}

static PARCSigningAlgorithm
_crc32Signer_GetSigningAlgorithm(_CRC32Signer *interfaceContext)
{
    return PARCSigningAlgorithm_NULL;
}

static PARCCryptoHashType
_crc32Signer_GetCryptoHashType(_CRC32Signer *interfaceContext)
{
    return PARCCryptoHashType_CRC32C;
}

static PARCCryptoHasher *
_crc32Signer_GetCryptoHasher(_CRC32Signer *signer)
{
    return signer->hasher;
}

static PARCCryptoHasher *
_crc32Verifier_GetCryptoHasher(_CRC32Verifier *verifier, PARCKeyId *keyid, PARCCryptoHashType hashType)
{
    assertTrue(hashType == PARCCryptoHashType_CRC32C, "Only supports PARCCryptoHashType_CRC32C, got request for %s", parcCryptoHashType_ToString(hashType));

    return verifier->hasher;
}

static bool
_crc32Verifier_VerifyDigest(_CRC32Verifier *verifier, PARCKeyId *keyid, PARCCryptoHash *locallyComputedHash,
                            PARCCryptoSuite suite, PARCSignature *signatureToVerify)
{
    assertTrue(suite == PARCCryptoSuite_NULL_CRC32C, "Only supports PARC_SUITE_NULL_CRC32C, got request for %d", suite);

    PARCBuffer *calculatedCrc = parcCryptoHash_GetDigest(locallyComputedHash);

    // the signature is the CRC, so we just need to compare to the to calculated CRC32C "hash"
    PARCBuffer *crcToVerify = parcSignature_GetSignature(signatureToVerify);

    return parcBuffer_Equals(calculatedCrc, crcToVerify);
}

static bool
_crc32Verifier_AllowedCryptoSuite(_CRC32Verifier *verifier, PARCKeyId *keyid, PARCCryptoSuite suite)
{
    return (suite == PARCCryptoSuite_NULL_CRC32C);
}

PARCSigningInterface *CRC32SignerAsPARCSigner = &(PARCSigningInterface) {
    .GetCryptoHasher = (PARCCryptoHasher * (*)(void *))_crc32Signer_GetCryptoHasher,
    .SignDigest = (PARCSignature * (*)(void *, const PARCCryptoHash *))_crc32Signer_SignDigest,
    .GetSigningAlgorithm = (PARCSigningAlgorithm (*)(void *))_crc32Signer_GetSigningAlgorithm,
    .GetCryptoHashType = (PARCCryptoHashType (*)(void *))_crc32Signer_GetCryptoHashType
};

PARCVerifierInterface *CRC32VerifierAsPARCVerifier = &(PARCVerifierInterface) {
    .GetCryptoHasher = (PARCCryptoHasher * (*)(void *, PARCKeyId *, PARCCryptoHashType))_crc32Verifier_GetCryptoHasher,
    .VerifyDigest = (bool (*)(void *, PARCKeyId *, PARCCryptoHash *, PARCCryptoSuite, PARCSignature *))_crc32Verifier_VerifyDigest,
    .AddKey = NULL,
    .RemoveKeyId = NULL,
    .AllowedCryptoSuite = (bool (*)(void *, PARCKeyId *, PARCCryptoSuite))_crc32Verifier_AllowedCryptoSuite,
};

static PARCSigner *
_crc32Signer_Create(void)
{
    _CRC32Signer *crc32Signer = parcObject_CreateInstance(_CRC32Signer);
    assertNotNull(crc32Signer, "parcObject_CreateInstance returned NULL");

    crc32Signer->hasher = parcCryptoHasher_Create(PARCCryptoHashType_CRC32C);
    PARCSigner *signer = parcSigner_Create(crc32Signer, CRC32SignerAsPARCSigner);
    _crc32CSigner_Release(&crc32Signer);

    return signer;
}

PARCSigner *
ccnxValidationCRC32C_CreateSigner(void)
{
    return _crc32Signer_Create();
}

static PARCVerifier *
_crc32Verifier_Create(void)
{
    _CRC32Verifier *crcVerifier = parcObject_CreateInstance(_CRC32Verifier);
    assertNotNull(crcVerifier, "parcObject_CreateInstance returned NULL");

    crcVerifier->hasher = parcCryptoHasher_Create(PARCCryptoHashType_CRC32C);

    PARCVerifier *verifier = parcVerifier_Create(crcVerifier, CRC32VerifierAsPARCVerifier);
    _crc32Verifier_Release(&crcVerifier);

    return verifier;
}

PARCVerifier *
ccnxValidationCRC32C_CreateVerifier(void)
{
    return _crc32Verifier_Create();
}
