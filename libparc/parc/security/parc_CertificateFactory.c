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
 * parc_CertificateFactory.c
 * PARC Library
 */

#include <config.h>

#include <parc/security/parc_CertificateFactory.h>
#include <parc/security/parc_X509Certificate.h>

#include <parc/algol/parc_Object.h>

struct parc_certificate_factory {
    PARCCertificateType type;
    PARCContainerEncoding encoding;
};

parcObject_ExtendPARCObject(PARCCertificateFactory, NULL, NULL, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementAcquire(parcCertificateFactory, PARCCertificateFactory);
parcObject_ImplementRelease(parcCertificateFactory, PARCCertificateFactory);

PARCCertificateFactory *
parcCertificateFactory_Create(PARCCertificateType type, PARCContainerEncoding encoding)
{
    PARCCertificateFactory *factory = parcObject_CreateInstance(PARCCertificateFactory);
    factory->type = type;
    factory->encoding = encoding;
    return factory;
}

PARCCertificate *
parcCertificateFactory_CreateCertificateFromFile(PARCCertificateFactory *factory, char *filename, char *password __attribute__((unused)))
{
    if (factory->type == PARCCertificateType_X509 && factory->encoding == PARCContainerEncoding_PEM) {
        PARCX509Certificate *certificate = parcX509Certificate_CreateFromPEMFile(filename);
        PARCCertificate *wrapper = parcCertificate_CreateFromInstance(PARCX509CertificateInterface, certificate);
        return wrapper;
    }

    // Unsupported configuration
    return NULL;
}

PARCCertificate *
parcCertificateFactory_CreateCertificateFromBuffer(PARCCertificateFactory *factory, PARCBuffer *buffer)
{
    if (factory->type == PARCCertificateType_X509 && factory->encoding == PARCContainerEncoding_DER) {
        PARCX509Certificate *certificate = parcX509Certificate_CreateFromDERBuffer(buffer);

        // This may fail.
        if (certificate == NULL) {
            return NULL;
        }

        PARCCertificate *wrapper = parcCertificate_CreateFromInstance(PARCX509CertificateInterface, certificate);
        return wrapper;
    }

    // Unsupported configuration
    return NULL;
}

PARCCertificate *
parcCertificateFactory_CreateSelfSignedCertificate(PARCCertificateFactory *factory, PARCBuffer **privateKey,
                                                   char *subjectName, PARCSigningAlgorithm signAlgo,
                                                   size_t keyLength, size_t valdityDays)
{
    if (factory->type == PARCCertificateType_X509 && factory->encoding == PARCContainerEncoding_DER) {
      PARCX509Certificate *certificate = NULL;
      switch (signAlgo)
      {
        case PARCSigningAlgorithm_RSA:
          certificate = parcX509Certificate_CreateSelfSignedCertificate(privateKey, subjectName, (int) keyLength, valdityDays, PARCKeyType_RSA);
          break;
        case PARCSigningAlgorithm_ECDSA:
          certificate = parcX509Certificate_CreateSelfSignedCertificate(privateKey, subjectName, (int) keyLength, valdityDays, PARCKeyType_EC);
          break;
      }

        // This may fail.
        if (certificate == NULL) {
            return NULL;
        }

        PARCCertificate *wrapper = parcCertificate_CreateFromInstance(PARCX509CertificateInterface, certificate);
        return wrapper;
    }

    // Unsupported configuration
    return NULL;
}
