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

#include <parc/assert/parc_Assert.h>

#include <parc/security/parc_KeyStore.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>

struct parc_key_store {
    PARCObject *instance;
    const PARCKeyStoreInterface *interface;
};

static bool
_parcKeyStore_Destructor(PARCKeyStore **keyStorePtr)
{
    PARCKeyStore *keyStore = *keyStorePtr;
    if (keyStore->interface != NULL && keyStore->instance != NULL) {
        parcObject_Release(&keyStore->instance);
    }

    return true;
}

parcObject_Override(PARCKeyStore, PARCObject,
                    .destructor = (PARCObjectDestructor *) _parcKeyStore_Destructor);

parcObject_ImplementAcquire(parcKeyStore, PARCKeyStore);
parcObject_ImplementRelease(parcKeyStore, PARCKeyStore);

PARCKeyStore *
parcKeyStore_Create(PARCObject *instance, const PARCKeyStoreInterface *interface)
{
    PARCKeyStore *keyStore = parcObject_CreateInstance(PARCKeyStore);

    if (keyStore != NULL) {
        keyStore->instance = parcObject_Acquire(instance);
        keyStore->interface = interface;
    }

    return keyStore;
}

PARCCryptoHash *
parcKeyStore_GetVerifierKeyDigest(const PARCKeyStore *interfaceContext)
{
    if (interfaceContext->interface != NULL) {
        return interfaceContext->interface->getVerifierKeyDigest(interfaceContext->instance);
    }
    return NULL;
}

PARCCryptoHash *
parcKeyStore_GetCertificateDigest(const PARCKeyStore *interfaceContext)
{
    if (interfaceContext->interface != NULL) {
        return interfaceContext->interface->getCertificateDigest(interfaceContext->instance);
    }
    return NULL;
}

PARCBuffer *
parcKeyStore_GetDEREncodedCertificate(const PARCKeyStore *interfaceContext)
{
    if (interfaceContext->interface != NULL) {
        return interfaceContext->interface->getDEREncodedCertificate(interfaceContext->instance);
    }
    return NULL;
}

PARCBuffer *
parcKeyStore_GetDEREncodedPublicKey(const PARCKeyStore *interfaceContext)
{
    if (interfaceContext->interface != NULL) {
        return interfaceContext->interface->getDEREncodedPublicKey(interfaceContext->instance);
    }
    return NULL;
}

PARCBuffer *
parcKeyStore_GetDEREncodedPrivateKey(const PARCKeyStore *interfaceContext)
{
    if (interfaceContext->interface != NULL) {
        return interfaceContext->interface->getDEREncodedPrivateKey(interfaceContext->instance);
    }
    return NULL;
}

PARCSigningAlgorithm
parcKeyStore_getSigningAlgorithm(const PARCKeyStore *interfaceContext)
{
    if (interfaceContext->interface != NULL) {
        return interfaceContext->interface->getSigningAlgorithm(interfaceContext->instance);
    }
    return PARCSigningAlgorithm_NULL;
}
