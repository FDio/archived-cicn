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
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include <pwd.h>
#include <unistd.h>

#include <sys/types.h>
#include <pwd.h>
#include <sys/stat.h>

#include <sys/param.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>
#include <ccnx/common/ccnx_KeystoreUtilities.h>
#include <parc/security/parc_Pkcs12KeyStore.h>
#include <parc/security/parc_PublicKeySigner.h>
#include <parc/security/parc_Signer.h>


struct keystore_params {
    char filename[1024];
    char password[1024];
    PARCSigner *signer;
};

#define OPT_KEYSTORE 'k'
#define OPT_PASSWORD 'p'
#define OPT_BITS     'b'
#define OPT_DAYS     'y'

static const char *emptyPassword = "";

static char *
ccnxKeystoreUtilities_ConstructPath(const char *dir, const char *file)
{
    char *path = parcMemory_AllocateAndClear(MAXPATHLEN);
    assertNotNull(path, "parcMemory_AllocateAndClear(%u) returned NULL", MAXPATHLEN);
    strncpy(path, dir, MAXPATHLEN);
    strncat(path, "/", MAXPATHLEN);
    strncat(path, file, MAXPATHLEN);
    return path;
}

static char *
ccnxKeystoreUtilities_HomeDirectoryFromEnv()
{
    char *home = getenv("HOME");
    if (home != NULL) {
        home = parcMemory_StringDuplicate(home, strlen(home) + 1);
    }
    return home;
}

static char *
ccnxKeystoreUtilities_HomeDirectoryFromPasswd()
{
    struct passwd *pw = getpwuid(getuid());
    const char *homedir = pw->pw_dir;
    return parcMemory_StringDuplicate(homedir, strlen(homedir) + 1);
}

static KeystoreParams *
ccnxKeystoreUtilities_OpenFromPath(const char *path, const char *password)
{
    KeystoreParams *params = NULL;

    // If the file exists, try to open it as a keystore
    struct stat filestat;
    int failure = stat(path, &filestat);
    if (!failure) {
        PARCPkcs12KeyStore *keyStore = parcPkcs12KeyStore_Open(path, password, PARCCryptoHashType_SHA256);
        PARCKeyStore *publicKeyStore = parcKeyStore_Create(keyStore, PARCPkcs12KeyStoreAsKeyStore);
        parcPkcs12KeyStore_Release(&keyStore);
        PARCPublicKeySigner *pksigner = parcPublicKeySigner_Create(publicKeyStore, PARCSigningAlgorithm_RSA, PARCCryptoHashType_SHA256);
        PARCSigner *signer = parcSigner_Create(pksigner, PARCPublicKeySignerAsSigner);
        parcPublicKeySigner_Release(&pksigner);

        if (signer) {
            params = ccnxKeystoreUtilities_Create(signer, path, password);

            parcSigner_Release(&signer);
            parcKeyStore_Release(&publicKeyStore);
        }
    }

    return params;
}

static KeystoreParams *
ccnxKeystoreUtilities_CreateInPath(const char *path, const char *password, int keystoreBits, int keystoreDays)
{
    KeystoreParams *params = NULL;

    bool success = parcPkcs12KeyStore_CreateFile(path, password, "ccnxuser", keystoreBits, keystoreDays);
    if (success) {
        PARCPkcs12KeyStore *keyStore = parcPkcs12KeyStore_Open(path, password, PARCCryptoHashType_SHA256);
        PARCKeyStore *publicKeyStore = parcKeyStore_Create(keyStore, PARCPkcs12KeyStoreAsKeyStore);
        parcPkcs12KeyStore_Release(&keyStore);
        PARCPublicKeySigner *pksigner = parcPublicKeySigner_Create(publicKeyStore, PARCSigningAlgorithm_RSA, PARCCryptoHashType_SHA256);
        PARCSigner *signer = parcSigner_Create(pksigner, PARCPublicKeySignerAsSigner);
        parcPublicKeySigner_Release(&pksigner);

        if (signer) {
            params = ccnxKeystoreUtilities_Create(signer, path, password);

            parcSigner_Release(&signer);
            parcKeyStore_Release(&publicKeyStore);
        }
    }
    return params;
}


static char *
ccnxKeystoreUtilities_GetHomeDirectory()
{
    char *homedir = ccnxKeystoreUtilities_HomeDirectoryFromEnv();
    if (homedir == NULL) {
        homedir = ccnxKeystoreUtilities_HomeDirectoryFromPasswd();
    }
    return homedir;
}

static KeystoreParams *
ccnxKeystoreUtilities_OpenFromHomeDirectory(const char *password)
{
    KeystoreParams *params = NULL;

    char *homedir = ccnxKeystoreUtilities_GetHomeDirectory();
    char *ccnxdir = ccnxKeystoreUtilities_ConstructPath(homedir, ".ccnx");


    char *path = ccnxKeystoreUtilities_ConstructPath(ccnxdir, ".ccnx_keystore.p12");
    params = ccnxKeystoreUtilities_OpenFromPath(path, password);
    parcMemory_Deallocate((void **) &path);

    if (params == NULL) {
        // try the older filename
        char *path = ccnxKeystoreUtilities_ConstructPath(ccnxdir, ".ccnx_keystore");
        params = ccnxKeystoreUtilities_OpenFromPath(path, password);
        parcMemory_Deallocate((void **) &path);
    }

    parcMemory_Deallocate((void **) &ccnxdir);
    parcMemory_Deallocate((void **) &homedir);
    return params;
}

static KeystoreParams *
ccnxKeystoreUtilities_CreateInHomeDirectory(const char *keystorePassword, int keystoreBits, int keystoreDays)
{
    char *homedir = ccnxKeystoreUtilities_GetHomeDirectory();
    char *ccnxdir = ccnxKeystoreUtilities_ConstructPath(homedir, ".ccnx");
    // Needs to check the return value to ensure that it was created. See case 1003.
    mkdir(ccnxdir, 0700);

    char *path = ccnxKeystoreUtilities_ConstructPath(ccnxdir, ".ccnx_keystore.p12");
    KeystoreParams *params = ccnxKeystoreUtilities_CreateInPath(path, keystorePassword, keystoreBits, keystoreDays);

    parcMemory_Deallocate((void **) &path);
    parcMemory_Deallocate((void **) &ccnxdir);
    parcMemory_Deallocate((void **) &homedir);
    return params;
}

#ifdef __ANDROID__
static char *
getpass(const char *prompt)
{
    trapNotImplemented("getpass() is not implemented on Android. See BugzId: 3864");
}
#endif

/**
 * Read a password, then zero the static memory
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return An allocated string, use <code>parcMemory_Deallocate()</code> on it.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static char *
_readPassword(const char *prompt)
{
    char *staticBuffer = getpass(prompt);
    char *password = parcMemory_StringDuplicate(staticBuffer, strlen(staticBuffer) + 1);
    memset(staticBuffer, 0, strlen(staticBuffer));
    return password;
}

KeystoreParams *
ccnxKeystoreUtilities_OpenFile(const char *keystoreFile, const char *keystorePassword)
{
    if (keystorePassword == NULL) {
        keystorePassword = emptyPassword;
    }

    KeystoreParams *params = NULL;

    if (keystoreFile == NULL) {
        params = ccnxKeystoreUtilities_OpenFromHomeDirectory(keystorePassword);
    } else {
        params = ccnxKeystoreUtilities_OpenFromPath(keystoreFile, keystorePassword);
    }

    return params;
}

KeystoreParams *
ccnxKeystoreUtilities_CreateFile(const char *keystoreFile, const char *keystorePassword, int keystoreBits, int keystoreDays)
{
    if (keystorePassword == NULL) {
        keystorePassword = emptyPassword;
    }

    KeystoreParams *params = NULL;

    if (keystoreFile == NULL) {
        params = ccnxKeystoreUtilities_CreateInHomeDirectory(keystorePassword, keystoreBits, keystoreDays);
    } else {
        params = ccnxKeystoreUtilities_CreateInPath(keystoreFile, keystorePassword, keystoreBits, keystoreDays);
    }

    return params;
}

KeystoreParams *
ccnxKeystoreUtilities_Create(PARCSigner *signer, const char *path, const char *password)
{
    KeystoreParams *params = parcMemory_AllocateAndClear(sizeof(KeystoreParams));
    assertNotNull(params, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(KeystoreParams));
    params->signer = parcSigner_Acquire(signer);
    strncpy(params->filename, path, 1024);
    strncpy(params->password, password, 1024);
    return params;
}

void
keystoreParams_Destroy(KeystoreParams **paramsPtr)
{
    KeystoreParams *params = *paramsPtr;
    if (params->signer != NULL) {
        parcSigner_Release(&params->signer);
    }
    parcMemory_Deallocate((void **) &params);
    *paramsPtr = NULL;
}

char *
ccnxKeystoreUtilities_ReadPassword(void)
{
    return _readPassword("Password: ");
}

bool
ccnxKeystoreUtilities_ConfirmPassword(const char *mustEqualPassword)
{
    bool equals = false;

    char *b = _readPassword("Confirm  : ");
    if (strcmp(mustEqualPassword, b) == 0) {
        equals = true;
    }

    memset(b, 0, strlen(b));
    parcMemory_Deallocate((void **) &b);
    return equals;
}

const char *
ccnxKeystoreUtilities_GetFileName(const KeystoreParams *params)
{
    return params->filename;
}

const char *
ccnxKeystoreUtilities_GetPassword(const KeystoreParams *params)
{
    return params->password;
}
