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
 * @file ccnx_KeystoreUtilities.h
 * @ingroup Signature
 * @brief A set of tools for working with the CCNx keystore.
 *
 */
#ifndef libccnx_ccnx_KeystoreUtilities_h
#define libccnx_ccnx_KeystoreUtilities_h

#include <parc/security/parc_Signer.h>

struct keystore_params;
/**
 * @typedef KeystoreParams
 * @brief Parameters for the KeyStore.
 */

typedef struct keystore_params KeystoreParams;

/**
 * Create a new `KeystoreParams` from a @p path, @p password, and a {@link PARCSigner}.
 *
 * @param [in] signer A pointer to an instance of `PARCSigner`.
 * @param [in] path The path to use.
 * @param [in] password The password to use.
 *
 * @return A pointer to a new instance of `KeystoreParams`.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 *
 */
KeystoreParams *ccnxKeystoreUtilities_Create(PARCSigner *signer, const char *path, const char *password);

/**
 * Destroy the `KeystoreParams`.
 *
 * @param [in,out] paramsPtr A pointer to the pointer to the `KeystoreParams` to destroy.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 *
 */
void keystoreParams_Destroy(KeystoreParams **paramsPtr);

/**
 * Opens a PKCS12 keystore for use with CCNx, or creates it if missing.
 *
 *   All options may be NULL.
 *      keystoreFile     is the filename and path to use.  If the option is omitted, then the default location is used.
 *                       The default location is in ~/.ccnx/.ccnx_keystore.p12, which is a PKCS12 keystore.  For compatability
 *                       with older implementations, will also look for ~/.ccnx/.ccnx_keystore without the file extension.
 *      keystorePassword is the password to use.  If missing, will prompt with getpass(3).
 *
 *   This function uses the equivalent of getopt_long(3).  It does not change the argv.
 *
 * @param [in] keystoreFile The full path to the keystore, may be NULL to use ~/.ccnx/.ccnx_keystore.p12
 * @param [in] keystorePassword The keystore password, may be NULL for no password.
 * @return The `KeystoreParams`, including the path used, password used, and the `PARCSigner`, NULL if cannot be opened.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
KeystoreParams *ccnxKeystoreUtilities_OpenFile(const char *keystoreFile, const char *keystorePassword);

/**
 * Creates a PKCS12 keystore.
 *
 * @param [in] keystoreFile may be NULL to use the default location
 * @param [in] keystorePassword The keystore password, may be NULL for no password.
 * @param [in] keystoreBits The keystore bits, may be NULL for no password.
 * @param [in] keystoreDays The keystore days, may be NULL for no password.
 * @return The keystore parameters, including the path used, password used, and the `PARCSigner`, NULL if cannot be created.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
KeystoreParams *ccnxKeystoreUtilities_CreateFile(const char *keystoreFile, const char *keystorePassword, int keystoreBits, int keystoreDays);

/**
 * Returns an allocated buffer with password
 *
 *   Reads a password from stdin, then scrubs the static memory
 *
 * @return Free with {@link parcMemory_Deallocate()}
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
char *ccnxKeystoreUtilities_ReadPassword(void);

/**
 * Returns an allocated buffer with password
 *
 *   Reads a password from stdin, then scrubs the static memory.
 *   Confirms that it equals the provided password.
 *
 * @param [in] mustEqualPassword The password that must match.
 *
 * @return `true` if the password from stdin matches.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool ccnxKeystoreUtilities_ConfirmPassword(const char *mustEqualPassword);

/**
 * Get the file name from the given {@link KeystoreParams} instance.
 *
 * @param [in] params A pointer to a valid `KeystoreParams` instance.
 *
 * @return A pointer to a null-terminated C string containing the file name.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 *
 */
const char *ccnxKeystoreUtilities_GetFileName(const KeystoreParams *params);

/**
 * Get the password from the given `KeyStoreParams` instance.
 *
 * @param [in] params A pointer to a valid `KeystoreParams` instance.
 *
 * @return A pointer to a null-terminated C string containing the password.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 *
 */
const char *ccnxKeystoreUtilities_GetPassword(const KeystoreParams *params);
#endif // libccnx_ccnx_KeystoreUtilities_h
