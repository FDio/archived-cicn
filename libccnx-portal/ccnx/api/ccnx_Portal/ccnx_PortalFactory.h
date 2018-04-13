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
 * @file ccnx_PortalFactory.h
 * @brief An API for creating Portals.
 *
 */
#ifndef CCNx_Portal_API_ccnx_PortalFactory_h
#define CCNx_Portal_API_ccnx_PortalFactory_h

#include <parc/security/parc_CryptoSuite.h>

struct CCNxPortalFactory;
/**
 * @typedef CCNxPortalFactory
 * @brief A Portal factory.
 */
typedef struct CCNxPortalFactory CCNxPortalFactory;

#include <parc/algol/parc_Properties.h>
#include <parc/security/parc_Identity.h>
#include <parc/security/parc_KeyId.h>

#include <ccnx/transport/common/transport.h>
#include <ccnx/transport/common/transport_MetaMessage.h>
#include <ccnx/api/ccnx_Portal/ccnx_Portal.h>

extern const char *CCNxPortalFactory_LocalRouterName;
extern const char *CCNxPortalFactory_LocalForwarder;
extern const char *CCNxPortalFactory_LocalRouterTimeout;

/**
 * Create a `CCNxPortalFactory` with the given {@link PARCIdentity}.
 *
 * If successful, a new reference to the given `PARCIdentity`
 * instance is acquired and released when the result is eventually released
 * via {@link ccnxPortalFactory_Release}.
 * The caller must release its original reference.
 *
 * The given `PARCIdentity` is the identity used for interacting with the Transport Framework when creating a new stack.
 * The default behaviour is that identity is also used for subsequent Content Object signing operations.
 * It is permissable, however, to set a different identity,
 * after a Portal API instance is created should its associated Transport Stack support setting a new identity.
 *
 * @param [in] identity A pointer to a `PARCIdentity` instance.
 *
 * @return non-NULL A pointer to a `CCNxPortalFactory` instance.
 * @return NULL Memory could not be allocated.
 *
 * Example:
 * @code
 * {
 *     char *subjectName = "An Example";
 *     char *keystoreName = "keystore";
 *     char *keystorePassword = "password";
 *
 *     bool success = parcPkcs12KeyStore_CreateFile(keystoreName, keystorePassword, subjectName, 1024, 30);
 *
 *     PARCIdentity *identity =
 *         parcIdentity_Create(parcIdentityFile_Create(keystoreName, keystorePassword), PARCIdentityFileAsPARCIdentity);
 *
 *     CCNxPortalFactory *factory = ccnxPortalFactory_Create(identity);
 *     parcIdentity_Release(&identity);
 *
 *     ccnxPortalFactory_Release(&factory);
 * }
 * @endcode
 *
 * @see {@link ccnxPortalFactory_Acquire}
 * @see {@link ccnxPortalFactory_Release}
 */
CCNxPortalFactory *ccnxPortalFactory_Create(const PARCIdentity *identity, PARCCryptoSuite suite);

/**
 * Print a human readable representation of the given `CCNxPortalFactory` instance.
 *
 * @param [in] factory A pointer to the instance of `CCNxPortalFactory` to display.
 * @param [in] indentation The level of indentation to use to pretty-print the output.
 *
 * Example:
 * @code
 * {
 *     CCNxPortalFactory *factory = ccnxPortalFactory_Create(...);
 *
 *     ccnxPortalFactory_Display(0, factory);
 *
 *     ccnxPortalFactory_Release(&factory);
 * }
 * @endcode
 *
 */
void ccnxPortalFactory_Display(const CCNxPortalFactory *factory, int indentation);

/**
 * Increase the number of references to a `CCNxPortalFactory` instance.
 *
 * Note that new `CCNxPortalFactory` is not created,
 * only that the given `CCNxPortalFactory` reference count is incremented.
 * Discard the reference by invoking `ccnxPortalFactory_Release`.
 *
 * @param [in] instance A pointer to a valid `CCNxPortalFactory`.
 *
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     CCNxPortalFactory *x = ccnxPortalFactory_Create(...);
 *
 *     CCNxPortalFactory *x2 = ccnxPortalFactory_Acquire(x);
 *
 *     ccnxPortalFactory_Release(&x);
 *     ccnxPortalFactory_Release(&x2);
 * }
 * @endcode
 *
 * @see {@link ccnxPortalFactory_Release}
 */
CCNxPortalFactory *ccnxPortalFactory_Acquire(const CCNxPortalFactory *instance);

/**
 * Release a previously acquired reference to the specified instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] factoryPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     CCNxPortalFactory *factory = ccnxPortalFactory_Create(...);
 *
 *     ccnxPortalFactory_Release(&factory);
 * }
 * @endcode
 */
void ccnxPortalFactory_Release(CCNxPortalFactory **factoryPtr);

/**
 * Get the {@link PARCIdentity} associated with the given `CCNxPortalFactory`.
 *
 * Note: a handle to the instance is not acquired, so you must not release it.
 *
 * @param [in] factory A pointer to a valid `CCNxPortalFactory`.
 *
 * @return A pointer to the internal `PARCIdentity` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxPortalFactory *factory = ccnxPortalFactory_Create(...);
 *
 *     PARCIdentity *identity = ccnxPortalFactory_GetIdentity(factory);
 *
 *     ccnxPortalFactory_Release(&factory);
 * }
 * @endcode
 *
 * @see {@link ccnxPortalFactory_Create}
 */
const PARCIdentity *ccnxPortalFactory_GetIdentity(const CCNxPortalFactory *factory);

/**
 * Get the `{@link PARCKeyId} of the {@link PARCIdentity} bound to the given CCNxPortalFactory.
 *
 * @param [in] factory A pointer to a valid `CCNxPortalFactory`.
 *
 * @return A pointer to the internal `PARCKeyId` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxPortalFactory *factory = ccnxPortalFactory_Create(...);
 *
 *     PARCKeyId *keyId = ccnxPortalFactory_GetKeyId(factory);
 *
 *     ccnxPortalFactory_Release(&factory);
 * }
 * @endcode
 *
 * @see {@link ccnxPortalFactory_Create}
 */
const PARCKeyId *ccnxPortalFactory_GetKeyId(const CCNxPortalFactory *factory);

/**
 * @typedef CCNxStackImpl
 * @brief A function that creates a `CCNxPortal` given a factory and attributes.
 */
typedef CCNxPortal *(CCNxStackImpl)(const CCNxPortalFactory *factory, const CCNxPortalAttributes *attributes);

/**
 * Create an {@link CCNxPortal} instance of the specified communication type, protocol and attributes.
 *
 * @param [in] factory A pointer to a CCNxPortal factory to use to create the instance.
 * @param [in,out] stackImplementation A pointer to a function initializing the protocol implementation.
 *
 * @return NULL An error occurred in creating the instance. See the value of `errno`
 * @return non-NULL A pointer to a valid `CCNxPortal` instance.
 *
 * Example:
 * @code
 * {
 *     CCNxPortal *portal = ccnxPortalFactory_CreatePortal(factory, ccnxPortalRTA_Message);
 *
 *     ccnxPortal_Release(&portal);
 * }
 * @endcode
 *
 * @see {@link ccnxPortal_Release}
 */
CCNxPortal *ccnxPortalFactory_CreatePortal(const CCNxPortalFactory *factory, CCNxStackImpl *stackImplementation);

PARCProperties *ccnxPortalFactory_GetProperties(const CCNxPortalFactory *factory);

const char *ccnxPortalFactory_GetProperty(const CCNxPortalFactory *factory, const char *restrict name, const char *restrict defaultValue);

void ccnxPortalFactory_SetProperty(const CCNxPortalFactory *factory, const char *restrict name, const char *restrict value);

#endif  // CCNx_Portal_API_ccnx_Portal_h

