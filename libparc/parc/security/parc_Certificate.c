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

/*
 *  parc_Certificate.c
 *  PARC Library
 */


#include <config.h>
#include <stdio.h>

#include <parc/security/parc_Certificate.h>
#include <parc/security/parc_CertificateType.h>
#include <parc/security/parc_ContainerEncoding.h>

#include <parc/algol/parc_Object.h>

struct parc_certificate {
    PARCCertificateInterface *interface;
    void *instance;
};

static void
_parcCertificate_FinalRelease(PARCCertificate **certP)
{
    PARCCertificate *certificate = (PARCCertificate *) *certP;

    if (certificate->instance != NULL) {
        parcObject_Release(&certificate->instance);
    }
}

parcObject_ExtendPARCObject(PARCCertificate, _parcCertificate_FinalRelease, NULL, NULL, NULL, NULL, NULL, NULL);

static PARCCertificate *
_parcCertificate_Create(PARCCertificateInterface *impl, void *instance)
{
    PARCCertificate *cert = parcObject_CreateInstance(PARCCertificate);
    cert->interface = impl;
    cert->instance = instance;
    return cert;
}

parcObject_ImplementAcquire(parcCertificate, PARCCertificate);

parcObject_ImplementRelease(parcCertificate, PARCCertificate);

PARCCertificate *
parcCertificate_CreateFromInstance(PARCCertificateInterface *impl, void *instance)
{
    return _parcCertificate_Create(impl, instance);
}

PARCCertificateType
parcCertificate_GetCertificateType(const PARCCertificate *cert)
{
    if (cert->interface->GetCertificateType != NULL) {
        return cert->interface->GetCertificateType(cert->instance);
    }
    return PARCCertificateType_Invalid;
}

PARCContainerEncoding
parcCertificate_GetContainerEncoding(const PARCCertificate *cert)
{
    if (cert->interface->GetContainerEncoding != NULL) {
        return cert->interface->GetContainerEncoding(cert->instance);
    }
    return PARCContainerEncoding_Invalid;
}

PARCCryptoHash *
parcCertificate_GetPublicKeyDigest(const PARCCertificate *certificate)
{
    if (certificate->interface->GetPublicKeyDigest != NULL) {
        return certificate->interface->GetPublicKeyDigest(certificate->instance);
    }
    return NULL;
}

PARCCryptoHash *
parcCertificate_GetCertificateDigest(const PARCCertificate *certificate)
{
    if (certificate->interface->GetCertificateDigest != NULL) {
        return certificate->interface->GetCertificateDigest(certificate->instance);
    }
    return NULL;
}

PARCBuffer *
parcCertificate_GetDEREncodedCertificate(const PARCCertificate *certificate)
{
    if (certificate->interface->GetDEREncodedCertificate != NULL) {
        return certificate->interface->GetDEREncodedCertificate(certificate->instance);
    }
    return NULL;
}

PARCBuffer *
parcCertificate_GetDEREncodedPublicKey(const PARCCertificate *certificate)
{
    if (certificate->interface->GetDEREncodedPublicKey != NULL) {
        return certificate->interface->GetDEREncodedPublicKey(certificate->instance);
    }
    return NULL;
}

PARCKey *
parcCertificate_GetPublicKey(const PARCCertificate *certificate)
{
    PARCBuffer *derEncodedVersion = parcCertificate_GetDEREncodedPublicKey(certificate);
    PARCCryptoHash *keyDigest = parcCertificate_GetPublicKeyDigest(certificate);
    PARCKeyId *keyId = parcKeyId_Create(parcCryptoHash_GetDigest(keyDigest));

    PARCKey *key = parcKey_CreateFromDerEncodedPublicKey(keyId, PARCSigningAlgorithm_RSA, derEncodedVersion);

    parcBuffer_Release(&derEncodedVersion);
    parcCryptoHash_Release(&keyDigest);
    parcKeyId_Release(&keyId);

    return key;
}
