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
#include <string.h>

#include <parc/assert/parc_Assert.h>

#include <parc/algol/parc_Object.h>
#include <parc/security/parc_Signature.h>
#include <parc/algol/parc_Memory.h>

struct parc_signature {
    PARCSigningAlgorithm signingAlgorithm;
    PARCCryptoHashType hashType;
    PARCBuffer *signatureBits;
};

static void
_parcSignature_FinalRelease(PARCSignature **signatureP)
{
    parcBuffer_Release(&(*signatureP)->signatureBits);
}

parcObject_ExtendPARCObject(PARCSignature, _parcSignature_FinalRelease, NULL, NULL, NULL, NULL, NULL, NULL);

PARCSignature *
parcSignature_Create(PARCSigningAlgorithm signingAlgorithm, PARCCryptoHashType hashType, PARCBuffer *signatureBits)
{
    parcAssertNotNull(signatureBits, "SignatureBits Parameter cannot be null");

    PARCSignature *signature = parcObject_CreateInstance(PARCSignature);
    parcAssertNotNull(signature, "parcObject_CreateInstance(%zu) returned NULL", sizeof(PARCSignature));

    signature->signingAlgorithm = signingAlgorithm;
    signature->hashType = hashType;
    signature->signatureBits = parcBuffer_Acquire(signatureBits);

    return signature;
}

parcObject_ImplementAcquire(parcSignature, PARCSignature);

parcObject_ImplementRelease(parcSignature, PARCSignature);


PARCSigningAlgorithm
parcSignature_GetSigningAlgorithm(const PARCSignature *signature)
{
    parcAssertNotNull(signature, "Parameter must be non-null");
    return signature->signingAlgorithm;
}

PARCCryptoHashType
parcSignature_GetHashType(const PARCSignature *signature)
{
    parcAssertNotNull(signature, "Parameter must be non-null");
    return signature->hashType;
}

PARCBuffer *
parcSignature_GetSignature(const PARCSignature *signature)
{
    parcAssertNotNull(signature, "Parameter must be non-null");
    return signature->signatureBits;
}

char *
parcSignature_ToString(const PARCSignature *signature)
{
    parcAssertNotNull(signature, "Parameter must be a non-null CCNxSignature pointer");

    char *bits = parcBuffer_ToString(signature->signatureBits);

    char *string;
    int nwritten = asprintf(&string, "CCNxSignedInfo { .signingAlg=%d, .digestAlg=%d, .signature=%s }",
                            signature->signingAlgorithm,
                            signature->hashType,
                            bits);
    parcAssertTrue(nwritten >= 0, "Error calling asprintf");

    parcMemory_Deallocate((void **) &bits);

    char *result = parcMemory_StringDuplicate(string, strlen(string));
    free(string);
    return result;
}

bool
parcSignature_Equals(const PARCSignature *x, const PARCSignature *y)
{
    if (x == y) {
        return true;
    }
    if (x == NULL || y == NULL) {
        return false;
    }

    if (x->signingAlgorithm == y->signingAlgorithm) {
        if (x->hashType == y->hashType) {
            if (parcBuffer_Equals(x->signatureBits, y->signatureBits)) {
                return true;
            }
        }
    }

    return false;
}
