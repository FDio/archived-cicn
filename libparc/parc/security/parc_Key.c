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

#include <string.h>
#include <stdio.h>

#include <parc/security/parc_Key.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_Memory.h>
#include <parc/assert/parc_Assert.h>

struct parc_key {
    PARCKeyId *keyid;
    PARCSigningAlgorithm signingAlg;
    PARCBuffer *key;
};

static void
_parcKey_FinalRelease(PARCKey **keyP)
{
    if ((*keyP)->keyid != NULL) {
        parcKeyId_Release(&(*keyP)->keyid);
    }
    if ((*keyP)->key != NULL) {
        parcBuffer_Release(&(*keyP)->key);
    }
}

parcObject_ExtendPARCObject(PARCKey, _parcKey_FinalRelease, NULL, NULL, NULL, NULL, NULL, NULL);

static PARCKey *
_parcKey_Create()
{
    PARCKey *key = parcObject_CreateInstance(PARCKey);
    return key;
}

/**
 * Create a Key for use with the specified signing algorithm.
 *
 * This method support Public Key alogirhtms
 *
 * For Public Key algorithms, the buffer should be a DER encoded key.
 */
PARCKey *
parcKey_CreateFromDerEncodedPublicKey(PARCKeyId *keyid, PARCSigningAlgorithm signingAlg, PARCBuffer *derEncodedKey)
{
    parcAssertNotNull(keyid, "Parameter keyid must be non-null");
    parcAssertNotNull(derEncodedKey, "Parameter derEncodedKey must be non-null");

    // Exclude the symmetric key algorithms
    switch (signingAlg) {
        case PARCSigningAlgorithm_RSA: // fallthrough
        case PARCSigningAlgorithm_DSA:
        case PARCSigningAlgorithm_ECDSA:
            break;

        default:
            parcTrapIllegalValueIf(true, "Unknown key algorithm or symmetric key algorithm: %s\n", parcSigningAlgorithm_ToString(signingAlg));
    }

    PARCKey *key = _parcKey_Create();
    parcAssertNotNull(key, "Unable to allocate memory for PARCKey");

    key->key = parcBuffer_Acquire(derEncodedKey);
    key->signingAlg = signingAlg;
    key->keyid = parcKeyId_Acquire(keyid);
    return key;
}

/**
 * Create a Key for use with the specified signing algorithm.
 *
 * This method support HMAC with symmetric keys.
 *
 * The secretkey is a set of random bytes.
 */
PARCKey *
parcKey_CreateFromSymmetricKey(PARCKeyId *keyid, PARCSigningAlgorithm signingAlg, PARCBuffer *secretkey)
{
    parcAssertNotNull(keyid, "Parameter keyid must be non-null");
    parcAssertNotNull(secretkey, "Parameter derEncodedKey must be non-null");

    // Exclude the symmetric key algorithms
    switch (signingAlg) {
        case PARCSigningAlgorithm_HMAC:
            break;

        default:
            parcTrapIllegalValueIf(true, "Unknown key algorithm or symmetric key algorithm: %s\n", parcSigningAlgorithm_ToString(signingAlg));
    }

    PARCKey *key = _parcKey_Create();
    parcAssertNotNull(key, "Unable to allocate memory for PARCKey");

    key->key = parcBuffer_Acquire(secretkey);
    key->signingAlg = signingAlg;
    key->keyid = parcKeyId_Acquire(keyid);
    return key;
}

/**
 * Destroys the key, keyid, and key byte buffer
 */

parcObject_ImplementAcquire(parcKey, PARCKey);

parcObject_ImplementRelease(parcKey, PARCKey);

void
parcKey_AssertValid(PARCKey *keyPtr)
{
    parcAssertNotNull(keyPtr, "Parameter must be non-null double pointer");
    parcAssertNotNull(keyPtr->key, "Parameter key must not be null");
    parcAssertNotNull(keyPtr->keyid, "Parameter keyId must not be null");
}

PARCKeyId *
parcKey_GetKeyId(const PARCKey *key)
{
    parcAssertNotNull(key, "Parameter must be non-null");
    return key->keyid;
}

PARCSigningAlgorithm
parcKey_GetSigningAlgorithm(const PARCKey *key)
{
    parcAssertNotNull(key, "Parameter must be non-null");
    return key->signingAlg;
}

PARCBuffer *
parcKey_GetKey(const PARCKey *key)
{
    parcAssertNotNull(key, "Parameter must be non-null");
    return key->key;
}

/**
 * keyA equals keyB iff the KeyIds are equal, the SigningAlgs are equal, and the keys are equal.
 * NULL equals NULL, but NULL does not equal any non-NULL
 */
bool
parcKey_Equals(const PARCKey *keyA, const PARCKey *keyB)
{
    if (keyA == keyB) {
        return true;
    }

    if (keyA == NULL || keyB == NULL) {
        return false;
    }

    if (keyA->signingAlg == keyB->signingAlg) {
        if (parcKeyId_Equals(keyA->keyid, keyB->keyid)) {
            if (parcBuffer_Equals(keyA->key, keyB->key)) {
                return true;
            }
        }
    }

    return false;
}

PARCKey *
parcKey_Copy(const PARCKey *original)
{
    PARCKey *newkey = _parcKey_Create();
    parcAssertNotNull(newkey, "Unable to allocate memory for a new key");
    newkey->key = parcBuffer_Copy(original->key);
    newkey->keyid = parcKeyId_Copy(original->keyid);
    newkey->signingAlg = original->signingAlg;
    return newkey;
}

char *
parcKey_ToString(const PARCKey *key)
{
    char *string;
    int failure = asprintf(&string, "PARCKey {.KeyID=\"%s\", .SigningAlgorithm=\"%s\" }",
                           parcKeyId_ToString(key->keyid),
                           parcSigningAlgorithm_ToString(key->signingAlg));
    parcAssertTrue(failure > -1, "Error asprintf");

    char *result = parcMemory_StringDuplicate(string, strlen(string));
    free(string);

    return result;
}
