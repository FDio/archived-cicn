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

#include <sys/stat.h>
#include <unistd.h>

#include <parc/security/parc_Identity.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/security/parc_Pkcs12KeyStore.h>
#include <parc/security/parc_PublicKeySigner.h>

#include <parc/security/parc_IdentityFile.h>

struct parc_identity_file {
    const char *fileName;
    const char *passWord;
};

PARCIdentityInterface *PARCIdentityFileAsPARCIdentity = &(PARCIdentityInterface) {
    .Acquire = (PARCIdentity * (*)(void *))parcIdentityFile_Acquire,
    .Release = (void (*)(void **))parcIdentityFile_Release,
    .GetPassWord = (void *(*)(const void *))parcIdentityFile_GetPassWord,
    .GetFileName = (void *(*)(const void *))parcIdentityFile_GetFileName,
    .GetSigner = (PARCSigner * (*)(const void *, PARCCryptoSuite))parcIdentityFile_CreateSigner,
    .Equals = (bool (*)(const void *, const void *))parcIdentityFile_Equals,
    .Display = (void (*)(const void *, size_t))parcIdentityFile_Display
};

void static
_finalize(PARCIdentityFile **IdentityPtr)
{
    PARCIdentityFile *identity = *IdentityPtr;
    parcMemory_Deallocate((void **) &(identity->fileName));
    parcMemory_Deallocate((void **) &(identity->passWord));
}


parcObject_ExtendPARCObject(PARCIdentityFile, _finalize, NULL, NULL, NULL, NULL, NULL, NULL);

PARCIdentityFile *
parcIdentityFile_Create(const char *fileName, const char *passWord)
{
    PARCIdentityFile *instance = parcObject_CreateInstance(PARCIdentityFile);

    if (instance != NULL) {
        instance->fileName = parcMemory_StringDuplicate(fileName, strlen(fileName));
        instance->passWord = parcMemory_StringDuplicate(passWord, strlen(passWord));
    }

    return instance;
}

parcObject_ImplementAcquire(parcIdentityFile, PARCIdentityFile);

parcObject_ImplementRelease(parcIdentityFile, PARCIdentityFile);

bool
parcIdentityFile_Exists(const PARCIdentityFile *identity)
{
    bool result = false;

    struct stat statbuf;

    if (stat(parcIdentityFile_GetFileName(identity), &statbuf) != -1) {
        if (S_ISREG(statbuf.st_mode)) {
            result = (access(parcIdentityFile_GetFileName(identity), F_OK | R_OK) == 0);
        }
    }

    return result;
}

const char *
parcIdentityFile_GetFileName(const PARCIdentityFile *identity)
{
    return identity->fileName;
}

const char *
parcIdentityFile_GetPassWord(const PARCIdentityFile *identity)
{
    return identity->passWord;
}

PARCSigner *
parcIdentityFile_CreateSigner(const PARCIdentityFile *identity, PARCCryptoSuite suite)
{
    PARCPkcs12KeyStore *keyStore = parcPkcs12KeyStore_Open(identity->fileName, identity->passWord, PARCCryptoHashType_SHA256);
    PARCKeyStore *publicKeyStore = parcKeyStore_Create(keyStore, PARCPkcs12KeyStoreAsKeyStore);
    parcPkcs12KeyStore_Release(&keyStore);

    PARCSigningAlgorithm signAlgo = parcKeyStore_getSigningAlgorithm(publicKeyStore);

    if (signAlgo != parcSigningAlgorithm_GetSigningAlgorithm(suite))
      return NULL;
    
    PARCPublicKeySigner *signer = parcPublicKeySigner_Create(publicKeyStore, suite);
    PARCSigner *pkSigner = parcSigner_Create(signer, PARCPublicKeySignerAsSigner);
    parcPublicKeySigner_Release(&signer);
    parcKeyStore_Release(&publicKeyStore);

    return pkSigner;
}

bool
parcIdentityFile_Equals(const PARCIdentityFile *a, const PARCIdentityFile *b)
{
    if (a == b) {
        return true;
    }
    if (a == NULL || b == NULL) {
        return false;
    }
    if (strcmp(parcIdentityFile_GetFileName(a), parcIdentityFile_GetFileName(b)) != 0) {
        return false;
    }
    if (strcmp(parcIdentityFile_GetPassWord(a), parcIdentityFile_GetPassWord(b)) != 0) {
        return false;
    }
    return true;
}

void
parcIdentityFile_Display(const PARCIdentityFile *instance, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCIdentityFile@%p {", instance);
    parcDisplayIndented_PrintLine(indentation + 1, ".fileName='%s', .passWord='%s'", instance->fileName, instance->passWord);
    parcDisplayIndented_PrintLine(indentation, "}", instance);
}
