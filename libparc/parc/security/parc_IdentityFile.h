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
 * @file parc_IdentityFile.h
 * @ingroup security
 * @brief This is a PARCIdentity represented as a PKCS12 keystore file.
 *
 * To create these files, use the `parc-publickey` command line tool. Or you could also
 * use `openssl` to create the same file.
 *
 * A `PARCIdentityFile` is a concrete instance of a `PARCIdentity`. To create a
 * `PARCIdentity` from a `PARCIdentityFile`, one would do the following:
 *
 * @code
 * {
 *     const char *keystoreName = "my_identity.p12";
 *     const char *keystorePassword = "my_password";
 *
 *     // Create the concrete identity instance from the PKCS12 file
 *     PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);
 *
 *     // Create a generic `PARCIdentity` from the concrete identity instance
 *     PARCIdentity *identity = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);
 *
 *     // Now use the `PARCIdentity` for signing purposes (amongst other things)
 *     PARCSigner *signer = parcIdentity_GetSigner(identity);
 * }
 * @endcode
 *
 */
#ifndef libparc_parc_IdentityFile_h
#define libparc_parc_IdentityFile_h

#include <parc/security/parc_Identity.h>

struct parc_identity_file;
typedef struct parc_identity_file PARCIdentityFile;

/**
 * The mapping of a PARCIdentityFile to the generic PARCIdentity.
 */
extern PARCIdentityInterface *PARCIdentityFileAsPARCIdentity;

/**
 * Create an instance of `PARCIdentityFile` from a given filename, and a password to unlock the stored information.
 *
 * The information is stored in PKCS 12 format.
 *
 * @param [in] fileName The name (relative path) to a file to be opened
 * @param [in] password The password to be used to open the identity file
 *
 * @return NULL A `PARCIdentityFile` could not be allocated.
 * @return PARCIdentityFile A newly allocated `PARCIdentityFile` that must be freed with `parcIdentityFile_Release()`
 *
 * Example:
 * @code
 * {
 *     PARCIdentityFile *file = parcIdentityFile_Create("./secret.p12", "1234");
 *     ...
 *     parcIdentityFile_Release(&file);
 * }
 * @endcode
 */
PARCIdentityFile *parcIdentityFile_Create(const char *fileName, const char *password);

/**
 * Increase the number of references to the given `PARCIdentityFile` instance.
 *
 * A new instance is not created,
 * only that the given instance's reference count is incremented.
 * Discard the acquired reference by invoking `parcIdentityFile_Release()`.
 *
 * @param [in] identity A pointer to a `PARCIdentityFile` instance.
 *
 * @return NULL An error occurred.
 * @return non-NULL A pointer to a PARCIdentityFile instance.
 *
 * Example:
 * @code
 * {
 *     PARCIdentityFile *file = parcIdentityFile_Create("./secret.p12", "1234");
 *     PARCIdentityFile *secondHandle = parcIdentityFile_Acquire(file);
 *     // use both handles as needed
 *     parcIdentityFile_Release(&file);
 *     parcIdentityFile_Release(&secondHandle);
 * }
 * @endcode
 */
PARCIdentityFile *parcIdentityFile_Acquire(const PARCIdentityFile *identity);

/**
 * Decrease the number of references to the given `PARCIdentityFile` instance.
 *
 * This only decrements the reference count so long as count >= 1. If the count
 * reaches zero, the object's memory is freed. The content pointer is always
 * NULLified after invocation.
 *
 * @param [in,out] identityPtr A pointer to a `PARCIdentityFile` instance.
 *
 * Example:
 * @code
 * {
 *     PARCIdentityFile *file = parcIdentityFile_Create("./secret.p12", "1234");
 *     // use handle as needed
 *     parcIdentityFile_Release(&file);
 * }
 * @endcode
 */
void parcIdentityFile_Release(PARCIdentityFile **identityPtr);

/**
 * Determine if the PARCIdentityFile exists.
 *
 * It must exist and be a regular file.
 *
 * @param [in] identity A pointer to a valid PARCIdentity instance.
 *
 * @return true The file exists and is a regular file.
 * @return false The file does not exist (see errno values for stat(2)) or is not a regular file.
 *
 * Example:
 * @code
 * {
 *     PARCIdentityFile *file = parcIdentityFile_Create("./secret.p12", "1234");
 *
 *     if (parcIdentityFile_Exists(file) == true) {
 *         printf("The file exists\n");
 *     } else {
 *         printf("The file does not exist fos\n");
 *     }
 * }
 * @endcode
 */
bool parcIdentityFile_Exists(const PARCIdentityFile *identity);

/**
 * Retrieve the name of the file associated with this `PARCIdentityFile` instance.
 *
 * @param [in] identity A pointer to a `PARCIdentityFile` instance.
 *
 * @return NULL An error occurred.
 * @return non-NULL A pointer to a PARCIdentityFile instance.
 *
 * Example:
 * @code
 * {
 *     PARCIdentityFile *file = parcIdentityFile_Create("./secret.p12", "1234");
 *     const char *fileName = parcIdentityFile_GetFileName(file);
 *     // use handle and/or file name as needed
 *     parcIdentityFile_Release(&file);
 * }
 * @endcode
 */
const char *parcIdentityFile_GetFileName(const PARCIdentityFile *identity);

/**
 * Retrieve the file password associated with this `PARCIdentityFile` instance.
 *
 * @param [in] identity A pointer to a `PARCIdentityFile` instance.
 *
 * @return NULL An error occurred.
 * @return non-NULL A pointer to a PARCIdentityFile instance.
 *
 * Example:
 * @code
 * {
 *     PARCIdentityFile *file = parcIdentityFile_Create("./secret.p12", "1234");
 *     const char *password = parcIdentityFile_GetPassWord(file);
 *     // use handle and/or file name as needed
 *     parcIdentityFile_Release(&file);
 * }
 * @endcode
 */
const char *parcIdentityFile_GetPassWord(const PARCIdentityFile *identity);

/**
 * Create an instance of `PARCSigner` from the given `PARCIdentity`
 *
 * @param [in] identity A pointer to a PARCIdentity instance.
 *
 * @return NULL An error occurred
 * @return PARCSigner A new `PARCSigner` instance created using the identity file for the public/private signing keys
 *
 * Example:
 * @code
 * {
 *     PARCIdentityFile *file = parcIdentityFile_Create("./secret.p12", "1234");
 *     PARCSigner *signer = parcIdentityFile_GetSigner(file);
 *     parcIdentityFile_Release(&file);
 *     // use the signer
 *     parcSigner_Release(&signer);
 * }
 * @endcode
 */
PARCSigner *parcIdentityFile_CreateSigner(const PARCIdentityFile *identity, PARCCryptoSuite suite);

/**
 * Determine if two PARCIdentityFiles are equal.
 *
 * The following equivalence relations on non-null `PARCIdentityFile` instances are maintained:
 *
 *   * It is reflexive: for any non-null reference value x, parcIdentityFile_Equals(x, x) must return true.
 *
 *   * It is symmetric: for any non-null reference values x and y, parcIdentityFile_Equals(x, y) must return true if and only if
 *        parcIdentityFile_Equals(y x) returns true.
 *
 *   * It is transitive: for any non-null reference values x, y, and z, if
 *        parcIdentityFile_Equals(x, y) returns true and
 *        parcIdentityFile_Equals(y, z) returns true,
 *        then  parcIdentityFile_Equals(x, z) must return true.
 *
 *   * It is consistent: for any non-null reference values x and y, multiple invocations of parcIdentityFile_Equals(x, y)
 *         consistently return true or consistently return false.
 *
 *   * For any non-null reference value x, parcIdentityFile_Equals(x, NULL)) must return false.
 *
 * @param a A pointer to a PARCIdentityFile instance.
 * @param b A pointer to a PARCIdentityFile instance.
 * @return True if the referenced PARCIdentityFiles are equal.
 *
 * Example:
 * @code
 * {
 *     PARCIdentityFile *a = parcIdentityFile_Create(...);
 *     PARCIdentityFile *b = parcIdentityFile_Create(...);
 *     if (parcIdentityFile_Equals(a, b)) {
 *         // this is expected
 *     } else {
 *         // this is not expected
 *     }
 *     parcIdentityFile_Release(&a);
 *     parcIdentityFile_Release(&b);
 * }
 *
 * }
 * @endcode
 */
bool parcIdentityFile_Equals(const PARCIdentityFile *a, const PARCIdentityFile *b);

/**
 * Print a human readable representation of the given `PARCIdentityFile`.
 *
 * @param [in] indentation The level of indentation to use to pretty-print the output.
 * @param [in] instance A pointer to the instance to display.
 *
 * Example:
 * @code
 * {
 *     PARCIdentityFile *instance = parcIdentityFile_Create("./secret.p12", "1234");
 *
 *     parcIdentityFile_Display(instance, 0);
 *
 *     parcIdentityFile_Release(&instance);
 * }
 * @endcode
 */
void parcIdentityFile_Display(const PARCIdentityFile *instance, int indentation);
#endif // libparc_parc_IdentityFile_h
