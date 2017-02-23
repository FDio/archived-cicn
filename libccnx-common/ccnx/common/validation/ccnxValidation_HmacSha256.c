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

#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>

#include <parc/security/parc_Verifier.h>
#include <parc/security/parc_SymmetricKeyStore.h>
#include <parc/security/parc_SymmetricKeySigner.h>

/**
 * Sets the Validation algorithm to HMAC with SHA-256 hash
 *
 * Sets the validation algorithm to be HMAC with a SHA-256 digest.  Optionally includes
 * a KeyId with the message.
 *
 * @param [in] message The message dictionary
 * @param [in] keyid (Optional) The KEYID to include the the message
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool
ccnxValidationHmacSha256_Set(CCNxTlvDictionary *message, const PARCBuffer *keyid)
{
    bool success = true;
    switch (ccnxTlvDictionary_GetSchemaVersion(message)) {
        case CCNxTlvDictionary_SchemaVersion_V1: {
            success &= ccnxTlvDictionary_PutInteger(message, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE, PARCCryptoSuite_HMAC_SHA256);

            if (keyid) {
                success &= ccnxTlvDictionary_PutBuffer(message, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_KEYID, keyid);
            }

            break;
        }

        default:
            trapIllegalValue(message, "Unknown schema version: %d", ccnxTlvDictionary_GetSchemaVersion(message));
    }
    return success;
}

bool
ccnxValidationHmacSha256_Test(const CCNxTlvDictionary *message)
{
    switch (ccnxTlvDictionary_GetSchemaVersion(message)) {
        case CCNxTlvDictionary_SchemaVersion_V1: {
            if (ccnxTlvDictionary_IsValueInteger(message, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE)) {
                uint64_t cryptosuite = ccnxTlvDictionary_GetInteger(message, CCNxCodecSchemaV1TlvDictionary_ValidationFastArray_CRYPTO_SUITE);
                return (cryptosuite == PARCCryptoSuite_HMAC_SHA256);
            }
            return false;
        }

        default:
            trapIllegalValue(message, "Unknown schema version: %d", ccnxTlvDictionary_GetSchemaVersion(message));
    }
    return false;
}

PARCSigner *
ccnxValidationHmacSha256_CreateSigner(PARCBuffer *secretKey)
{
    PARCSymmetricKeyStore *keyStore = parcSymmetricKeyStore_Create(secretKey);
    PARCSymmetricKeySigner *symmetricSigner = parcSymmetricKeySigner_Create(keyStore, PARCCryptoHashType_SHA256);
    parcSymmetricKeyStore_Release(&keyStore);

    PARCSigner *signer = parcSigner_Create(symmetricSigner, PARCSymmetricKeySignerAsSigner);
    parcSymmetricKeySigner_Release(&symmetricSigner);

    return signer;
}

PARCVerifier *
ccnxValidationHmacSha256_CreateVerifier(PARCBuffer *secretKey)
{
    trapNotImplemented("not finished yet");
}
