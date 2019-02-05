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

#include <config.h>
#include <stdio.h>
#include <LongBow/unit-test.h>

#include "../parc_CertificateFactory.c"
#include <parc/security/parc_X509Certificate.h>
#include <parc/algol/parc_SafeMemory.h>
#include <parc/security/parc_Security.h>
#include <parc/security/parc_CryptoHashType.h>
#include <parc/algol/parc_Buffer.h>

LONGBOW_TEST_RUNNER(parc_CertificateFactory)
{
    LONGBOW_RUN_TEST_FIXTURE(Global);
}

LONGBOW_TEST_RUNNER_SETUP(parc_CertificateFactory)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

// The Test Runner calls this function once after all the Test Fixtures are run.
LONGBOW_TEST_RUNNER_TEARDOWN(parc_CertificateFactory)
{
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE(Global)
{
    LONGBOW_RUN_TEST_CASE(Global, parc_CertificateFactory_AcquireRelease);
    LONGBOW_RUN_TEST_CASE(Global, parc_CertificateFactory_Create);
    LONGBOW_RUN_TEST_CASE(Global, parc_CertificateFactory_CreateFromFile);
    LONGBOW_RUN_TEST_CASE(Global, parc_CertificateFactory_CreateFromBuffer);
}

LONGBOW_TEST_FIXTURE_SETUP(Global)
{
    parcSecurity_Init();
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_FIXTURE_TEARDOWN(Global)
{
    parcSecurity_Fini();
    if (parcSafeMemory_ReportAllocation(STDOUT_FILENO) != 0) {
        printf("('%s' leaks memory by %d (allocs - frees)) ", longBowTestCase_GetName(testCase), parcMemory_Outstanding());
        return LONGBOW_STATUS_MEMORYLEAK;
    }
    return LONGBOW_STATUS_SUCCEEDED;
}

LONGBOW_TEST_CASE(Global, parc_CertificateFactory_AcquireRelease)
{
    PARCCertificateFactory *factory = parcCertificateFactory_Create(PARCCertificateType_X509, PARCContainerEncoding_PEM);
    assertNotNull(factory, "Expected non-NULL factory");

    PARCReferenceCount firstCount = parcObject_GetReferenceCount(factory);
    PARCCertificateFactory *copy = parcCertificateFactory_Acquire(factory);
    PARCReferenceCount secondCount = parcObject_GetReferenceCount(copy);

    assertTrue(firstCount == (secondCount - 1), "Expected incremented reference count after Acquire");

    parcCertificateFactory_Release(&copy);
    PARCReferenceCount thirdCount = parcObject_GetReferenceCount(factory);

    assertTrue(firstCount == thirdCount, "Expected equal reference counts after Release");

    parcCertificateFactory_Release(&factory);
}

LONGBOW_TEST_CASE(Global, parc_CertificateFactory_Create)
{
    PARCCertificateFactory *factory = parcCertificateFactory_Create(PARCCertificateType_X509, PARCContainerEncoding_PEM);
    assertNotNull(factory, "Expected non-NULL factory");
    assertTrue(factory->encoding == PARCContainerEncoding_PEM, "Expected PARCContainerEncoding_PEM (%d) encoding, got %d",
               PARCContainerEncoding_PEM, factory->encoding);
    assertTrue(factory->type == PARCCertificateType_X509, "Expected PARCCertificateType_X509 (%d) type, got %d",
               PARCCertificateType_X509, factory->type);
    parcCertificateFactory_Release(&factory);
}

LONGBOW_TEST_CASE(Global, parc_CertificateFactory_CreateFromFile)
{
    PARCCertificateFactory *factory = parcCertificateFactory_Create(PARCCertificateType_X509, PARCContainerEncoding_PEM);
    assertNotNull(factory, "Expected non-NULL factory");

    char *filename = "test.pem";
    PARCCertificate *certificate = parcCertificateFactory_CreateCertificateFromFile(factory, filename, NULL);

    assertNotNull(certificate, "Expected non-NULL certificate");
    assertTrue(parcCertificate_GetContainerEncoding(certificate) == PARCContainerEncoding_PEM,
               "Expected PARCContainerEncoding_PEM encoding, got %d", parcCertificate_GetContainerEncoding(certificate));
    assertTrue(parcCertificate_GetCertificateType(certificate) == PARCCertificateType_X509,
               "Expected PARCCertificateType_X509 type, got %d", parcCertificate_GetCertificateType(certificate));

    parcCertificate_Release(&certificate);
    parcCertificateFactory_Release(&factory);

    factory = parcCertificateFactory_Create(PARCCertificateType_Invalid, PARCContainerEncoding_PEM);

    PARCCertificate *nullCertificate = parcCertificateFactory_CreateCertificateFromFile(factory, filename, NULL);
    assertNull(nullCertificate, "Expected NULL certificate to be returned from factory with an unsupported configuration");

    parcCertificateFactory_Release(&factory);
}

LONGBOW_TEST_CASE(Global, parc_CertificateFactory_CreateFromBuffer)
{
    PARCCertificateFactory *factory = parcCertificateFactory_Create(PARCCertificateType_X509, PARCContainerEncoding_DER);
    assertNotNull(factory, "Expected non-NULL factory");

    char *filename = "test.pem";
    PARCX509Certificate *realCertificate = parcX509Certificate_CreateFromPEMFile(filename);
    PARCBuffer *certificateBuffer = parcX509Certificate_GetDEREncodedCertificate(realCertificate);

    PARCCertificate *certificate = parcCertificateFactory_CreateCertificateFromBuffer(factory, certificateBuffer);

    assertNotNull(certificate, "Expected non-NULL certificate");
    assertTrue(parcCertificate_GetContainerEncoding(certificate) == PARCContainerEncoding_DER,
               "Expected PARCContainerEncoding_DER encoding, got %d", parcCertificate_GetContainerEncoding(certificate));
    assertTrue(parcCertificate_GetCertificateType(certificate) == PARCCertificateType_X509,
               "Expected PARCCertificateType_X509 type, got %d", parcCertificate_GetCertificateType(certificate));

    parcCertificate_Release(&certificate);
    parcCertificateFactory_Release(&factory);
    parcX509Certificate_Release(&realCertificate);

    factory = parcCertificateFactory_Create(PARCCertificateType_Invalid, PARCContainerEncoding_PEM);

    PARCBuffer *invalid = parcBuffer_Allocate(10);
    PARCCertificate *nullCertificate = parcCertificateFactory_CreateCertificateFromBuffer(factory, invalid);
    assertNull(nullCertificate, "Expected NULL certificate to be returned from factory with an unsupported configuration");

    parcBuffer_Release(&invalid);
    parcCertificateFactory_Release(&factory);
}

int
main(int argc, char *argv[argc])
{
    LongBowRunner *testRunner = LONGBOW_TEST_RUNNER_CREATE(parc_CertificateFactory);
    int exitStatus = LONGBOW_TEST_MAIN(argc, argv, testRunner);
    longBowTestRunner_Destroy(&testRunner);
    exit(exitStatus);
}
