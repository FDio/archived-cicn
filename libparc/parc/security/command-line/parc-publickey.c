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
#include <errno.h>

#include <parc/assert/parc_Assert.h>

#include <parc/algol/parc_ArrayList.h>
#include <parc/security/parc_Security.h>
#include <parc/security/parc_Pkcs12KeyStore.h>
#include <parc/security/parc_PublicKeySigner.h>

void
parcPublicKey_Create(PARCArrayList *args)
{
    unsigned int keyLength = 1024;
    unsigned int validityDays = 30;

    char *fileName = parcArrayList_Get(args, 2);
    char *password = parcArrayList_Get(args, 3);
    char *subjectName = parcArrayList_Get(args, 4);
    PARCSigningAlgorithm signAlgo = PARCSigningAlgorithm_RSA;

    if (parcArrayList_Size(args) > 5) {
        keyLength = (unsigned int) strtoul(parcArrayList_Get(args, 5), NULL, 10);
    }

    if (parcArrayList_Size(args) > 6) {
        validityDays = (unsigned int) strtoul(parcArrayList_Get(args, 6), NULL, 10);
    }

    bool result = parcPkcs12KeyStore_CreateFile(fileName, password, subjectName, signAlgo, keyLength, validityDays);
    if (!result) {
        printf("Error: %s %s", fileName, strerror(errno));
        return;
    }
    printf("Created %s, key length %d valid for %d days.\n", fileName, keyLength, validityDays);
}

void
parcPublicKey_Validate(PARCArrayList *args)
{
    char *fileName = parcArrayList_Get(args, 2);
    char *password = parcArrayList_Get(args, 3);

    PARCPkcs12KeyStore *keyStore = parcPkcs12KeyStore_Open(fileName, password, PARCCryptoHashType_SHA256);
    PARCKeyStore *publicKeyStore = parcKeyStore_Create(keyStore, PARCPkcs12KeyStoreAsKeyStore);

    PARCPublicKeySigner *signer = parcPublicKeySigner_Create(publicKeyStore, PARCCryptoSuite_RSA_SHA256);
    PARCSigner *pkSigner = parcSigner_Create(signer, PARCPublicKeySignerAsSigner);

    parcKeyStore_Release(&publicKeyStore);
    parcPkcs12KeyStore_Release(&keyStore);

    if (pkSigner == NULL) {
        printf("Invalid %s\n", fileName);
        return;
    }
    printf("Valid %s\n", fileName);
}

void
printUsage(char *progName)
{
    printf("usage: %s [-h | --help] [[-c | --create] fileName password subjectName [keyLength validityDays] | [-v | --validate] fileName password]\n", progName);
    printf("\n");
    printf("\n");
    printf("Create and validate PKCS12 keystores that are used with the CCNx code.\n");
    printf("\n");
    printf("optional arguments:\n");
    printf("\t-h, --help\tShow this help message and exit\n");
    printf("\t-c, --create\tCreate a PKCS12 keystore with the given filename, password, subject name, and optional key length and validity length (in days)\n");
    printf("\n");
    printf("\t\t\texample: ./parc_publickey -c keyfile.pkcs12 <password> <subject name> 1024 365\n");
    printf("\n");
    printf("\t-v, --validate\tValidate a PKCS12 file with the given password\n");
    printf("\n");
    printf("\t\t\texample: ./parc_publickey -v keyfile.pkcs12 <password>");
    printf("\n");
}

int
main(int argc, char *argv[])
{
    char *programName = "parc_publickey";
    if (argc < 2) {
        printUsage(programName);
        exit(1);
    }

    PARCArrayList *args = parcArrayList_Create(NULL);
    parcArrayList_AddAll(args, (void **) argv, argc);

    parcSecurity_Init();

    char *arg = parcArrayList_Get(args, 1);
    if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
        printUsage(programName);
        return 0;
    } else if (strcmp(arg, "-c") == 0 || strcmp(arg, "--create") == 0) {
        parcPublicKey_Create(args);
    } else if (strcmp(arg, "-v") == 0 || strcmp(arg, "--validate") == 0) {
        parcPublicKey_Validate(args);
    } else {
        printUsage(programName);
        exit(1);
    }

    parcSecurity_Fini();
    return 0;
}
