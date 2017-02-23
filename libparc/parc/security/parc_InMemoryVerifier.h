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
 * @file parc_InMemoryVerifier.h
 * @ingroup security
 * @brief In memory verifier
 *
 */
#ifndef libparc_parc_InMemoryVerifier_h
#define libparc_parc_InMemoryVerifier_h

#include <parc/security/parc_Verifier.h>

struct parc_inmemory_verifier;
typedef struct parc_inmemory_verifier PARCInMemoryVerifier;

/**
 * Create an empty verifier.   It's destroyed via the PARCVerifierInterface->Destroy call.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCInMemoryVerifier *parcInMemoryVerifier_Create(void);

/**
 * Increase the number of references to the given `PARCInMemoryVerifier` instance.
 *
 * A new instance is not created,
 * only that the given instance's reference count is incremented.
 * Discard the acquired reference by invoking `parcInMemoryVerifier_Release()`.
 *
 * @param [in] signer A pointer to a `PARCInMemoryVerifier` instance.
 *
 * @return NULL An error occurred.
 * @return non-NULL A pointer to a PARCInMemoryVerifier instance.
 *
 * Example:
 * @code
 * {
 *      PARCInMemoryVerifier *verifier = parcInMemoryVerifier_Create();
 *      PARCInMemoryVerifier *handle = parcInMemoryVerifier_Acquire(signer);
 *      // use the handle instance as needed
 * }
 * @endcode
 */
PARCInMemoryVerifier *parcInMemoryVerifier_Acquire(const PARCInMemoryVerifier *verifier);

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
 * The contents of the dealloced memory used for the PARC object are undefined.
 * Do not reference the object after the last release.
 *
 * @param [in,out] verifierPtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCInMemoryVerifier *verifier = parcInMemoryVerifier_Create();
 *
 *     parcInMemoryVerifier_Release(&verifier);
 * }
 * @endcode
 */
void parcInMemoryVerifier_Release(PARCInMemoryVerifier **verifierPtr);
#endif // libparc_parc_InMemoryVerifier_h
