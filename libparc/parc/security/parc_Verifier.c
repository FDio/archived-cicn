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

#include <parc/assert/parc_Assert.h>

#include <parc/security/parc_Verifier.h>
#include <parc/algol/parc_Memory.h>

struct parc_verifier {
    PARCObject *instance;
    PARCVerifierInterface *interface;
};

static bool
_parcVerifier_FinalRelease(PARCVerifier **verifierPtr)
{
    PARCVerifier *verifier = *verifierPtr;
    if (verifier->instance != NULL) {
        parcObject_Release(&(verifier->instance));
    }
    return true;
}

void
parcVerifier_AssertValid(const PARCVerifier *verifier)
{
    parcAssertNotNull(verifier, "Parameter must be non-null PARCVerifier");
}

parcObject_ImplementAcquire(parcVerifier, PARCVerifier);
parcObject_ImplementRelease(parcVerifier, PARCVerifier);

parcObject_Override(PARCVerifier, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcVerifier_FinalRelease);

PARCVerifier *
parcVerifier_Create(PARCObject *instance, PARCVerifierInterface *interfaceContext)
{
    parcAssertNotNull(interfaceContext, "Parameter `interfaceContext` must be non-null interface pointer");
    parcAssertNotNull(instance, "Parameter `instance` must be non-null PARCObject pointer");

    PARCVerifier *verifier = parcObject_CreateInstance(PARCVerifier);
    parcAssertNotNull(verifier, "parcObject_CreateInstance returned NULL");

    verifier->instance = parcObject_Acquire(instance);
    verifier->interface = interfaceContext;

    return verifier;
}

bool
parcVerifier_VerifyDigestSignature(PARCVerifier *verifier, PARCKeyId *keyid, PARCCryptoHash *locallyComputedHash,
                                   PARCCryptoSuite suite, PARCSignature *signatureToVerify)
{
    parcAssertNotNull(verifier, "Parameter must be non-null PARCVerifier");
    parcAssertNotNull(locallyComputedHash, "cryptoHash to verify must not be null");
    parcAssertNotNull(signatureToVerify, "Signature to verify must not be null");

    // null keyid is allowed now that we support CRCs, etc.

    return verifier->interface->VerifyDigest(verifier->instance, keyid, locallyComputedHash, suite, signatureToVerify);
}

bool
parcVerifier_AllowedCryptoSuite(PARCVerifier *verifier, PARCKeyId *keyid, PARCCryptoSuite suite)
{
    parcAssertNotNull(verifier, "Parameter must be non-null PARCVerifier");
    return verifier->interface->AllowedCryptoSuite(verifier->instance, keyid, suite);
}

PARCCryptoHasher*
parcVerifier_GetCryptoHasher(PARCVerifier *verifier, PARCKeyId *keyid, PARCCryptoHashType hashType)
{
    parcAssertNotNull(verifier, "Parameter must be non-null PARCVerifier");
    return verifier->interface->GetCryptoHasher(verifier->instance, keyid, hashType);
}

void
parcVerifier_AddKey(PARCVerifier *verifier, PARCKey *key)
{
    parcAssertNotNull(verifier, "Parameter must be non-null PARCVerifier");
    verifier->interface->AddKey(verifier->instance, key);
}

void
parcVerifier_RemoveKeyId(PARCVerifier *verifier, PARCKeyId *keyid)
{
    parcAssertNotNull(verifier, "Parameter must be non-null PARCVerifier");
    verifier->interface->RemoveKeyId(verifier->instance, keyid);
}
