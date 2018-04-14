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

#include <LongBow/runtime.h>

#include <parc/security/parc_CryptoSuite.h>
#include <parc/security/parc_SigningAlgorithm.h>

PARCCryptoHashType
parcCryptoSuite_GetCryptoHash(PARCCryptoSuite suite)
{
    switch (suite) {
        case PARCCryptoSuite_DSA_SHA256:      // fallthrough
        case PARCCryptoSuite_HMAC_SHA256:     // fallthrough
        case PARCCryptoSuite_RSA_SHA256:      // fallthrough
        case PARCCryptoSuite_ECDSA_SHA256:
            return PARCCryptoHashType_SHA256;

        case PARCCryptoSuite_HMAC_SHA512:     // fallthrough
        case PARCCryptoSuite_RSA_SHA512:
            return PARCCryptoHashType_SHA512;

        case PARCCryptoSuite_NULL_CRC32C:
            return PARCCryptoHashType_CRC32C;

        default:
            trapIllegalValue(suite, "Unknown crypto suite: %d", suite);
    }
}

int
parcCryptoSuite_GetSignatureSizeBits(PARCCryptoSuite suite, int keyLengthBits)
{
    switch (suite) {
        case PARCCryptoSuite_DSA_SHA256:      // fallthrough
        case PARCCryptoSuite_RSA_SHA256:      // fallthrough
        case PARCCryptoSuite_RSA_SHA512:
            return keyLengthBits;

        case PARCCryptoSuite_ECDSA_SHA256:
            return keyLengthBits*2 + 64; //Overhead added by ECDSA 

        case PARCCryptoSuite_HMAC_SHA256:     // fallthrough
        case PARCCryptoSuite_HMAC_SHA512:     // fallthrough
            return 512;

        case PARCCryptoSuite_NULL_CRC32C:
            return 32;

        default:
            trapIllegalValue(suite, "Unknown crypto suite: %d", suite);
    }
}

int
parcCryptoSuite_GetSignatureSizeBytes(PARCCryptoSuite suite, int keyLengthBits)
{
    int keyLengthBytes = keyLengthBits >> 3;
    switch (suite) {
        case PARCCryptoSuite_DSA_SHA256:      // fallthrough
        case PARCCryptoSuite_RSA_SHA256:      // fallthrough
        case PARCCryptoSuite_RSA_SHA512:
            return keyLengthBytes;

        case PARCCryptoSuite_ECDSA_SHA256:
            return keyLengthBytes*2 + 8; //Overhead added by ECDSA 

        case PARCCryptoSuite_HMAC_SHA256:     // fallthrough
        case PARCCryptoSuite_HMAC_SHA512:     // fallthrough
            return 64;

        case PARCCryptoSuite_NULL_CRC32C:
            return 4;

        default:
            trapIllegalValue(suite, "Unknown crypto suite: %d", suite);
    }
}

PARCCryptoSuite parcCryptoSuite_GetFromSigningHash(PARCSigningAlgorithm signAlgo, PARCCryptoHashType hash) {

  switch (signAlgo) {
    case PARCSigningAlgorithm_DSA:
      return PARCCryptoSuite_DSA_SHA256 + hash -1;
    case PARCSigningAlgorithm_RSA:
      return PARCCryptoSuite_RSA_SHA256 + hash -1;
    case PARCSigningAlgorithm_ECDSA:
      return PARCCryptoSuite_ECDSA_SHA256 + hash -1;      
    case PARCSigningAlgorithm_NULL:
      return PARCCryptoSuite_NULL_CRC32C;
    default:
      trapIllegalValue(suite, "Unknown signing algorithm suite: %d", signAlgo);
  }
}

PARCSigningAlgorithm
parcCryptoSuite_GetSigningAlgorithm(PARCCryptoSuite suite)
{
    switch (suite) {
        case PARCCryptoSuite_DSA_SHA256:
            return PARCSigningAlgorithm_DSA;

        case PARCCryptoSuite_RSA_SHA256:      // fallthrough
        case PARCCryptoSuite_RSA_SHA512:
            return PARCSigningAlgorithm_RSA;

        case PARCCryptoSuite_HMAC_SHA256:     // fallthrough
        case PARCCryptoSuite_HMAC_SHA512:
            return PARCSigningAlgorithm_HMAC;

        case PARCCryptoSuite_ECDSA_SHA256:
	    return PARCSigningAlgorithm_ECDSA;
        case PARCCryptoSuite_NULL_CRC32C:
            return PARCSigningAlgorithm_NULL;

        default:
            trapIllegalValue(suit, "Unknown crypto suite: %d", suite);
    }
}
