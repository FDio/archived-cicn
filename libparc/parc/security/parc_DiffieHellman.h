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
 * @file parc_DiffieHellman.h
 * @ingroup security
 * @brief A factory for Diffie Hellman parameters.
 *
 */
#ifndef libparc_parc_DiffieHellman_h
#define libparc_parc_DiffieHellman_h

#include <parc/security/parc_DiffieHellmanGroup.h>
#include <parc/security/parc_DiffieHellmanKeyShare.h>

struct parc_diffie_hellman;
typedef struct parc_diffie_hellman PARCDiffieHellman;

/**
 * Create an instance of `PARCDiffieHellman.` that generates Diffie Hellman shares
 * for the specified key exchange mechanisms.
 *
 * @param [in] groupType A type of PARCDiffieHellmanGroup
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a `PARCDiffieHellman` instance.
 *
 * Example:
 * @code
 * {
 *     PARCDiffieHellman *dh = parcDiffieHellman_Create(PARCDiffieHellmanGroup_Secp521r1);
 *
 *     parcDiffieHellman_Release(&dh);
 * }
 * @endcode
 */
PARCDiffieHellman *parcDiffieHellman_Create(PARCDiffieHellmanGroup groupType);

/**
 * Increase the number of references to an instance of this object.
 *
 * Note that new instance is not created, only that the given instance's reference count
 * is incremented. Discard the reference by invoking `parcDiffieHellman_Release()`.
 *
 * @param [in] dh A `PARCDiffieHellman` instance.
 *
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     PARCDiffieHellman *dh = parcDiffieHellman_Acquire(dhInstance);
 *
 *     parcDiffieHellman_Release(&dh);
 * }
 * @endcode
 *
 * @see parcDiffieHellman_Release
 */
PARCDiffieHellman *parcDiffieHellman_Acquire(const PARCDiffieHellman *dh);

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
 * @param [in,out] dhP A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCDiffieHellman *dh = parcDiffieHellman_Create(PARCDiffieHellmanGroup_Secp521r1);
 *
 *     parcDiffieHellman_Release(&dh);
 * }
 * @endcode
 */
void parcDiffieHellman_Release(PARCDiffieHellman **dhP);

/*
 * Generate a fresh Diffie Hellman key share.
 *
 * @param [in] dh A `PARCDiffieHellman` instance.
 *
 * @return A `PARCDiffieHellmanKeyShare` instnace.
 *
 * Example:
 * @code
 * {
 *     PARCDiffieHellman *dh = parcDiffieHellman_Create(PARCDiffieHellmanGroup_Secp521r1);
 *
 *     PARCDiffieHellmanKeyShare *keyShare = parcDiffieHellman_GenerateKeyShare(dh);
 *     // use the key share
 *
 *     parcDiffieHellmanKeyShare_Release(&keyShare);
 * }
 * @endcode
 */
PARCDiffieHellmanKeyShare *parcDiffieHellman_GenerateKeyShare(PARCDiffieHellman *dh);
#endif // libparc_parc_DiffieHellman_h
