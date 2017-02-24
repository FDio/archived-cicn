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
 * @file parc_DiffieHellmanKeyShare.h
 * @ingroup security
 * @brief A Diffie Hellman key share.
 *
 */
#ifndef libparc_parc_DiffieHellmanKeyShare_h
#define libparc_parc_DiffieHellmanKeyShare_h

#include <parc/security/parc_DiffieHellmanGroup.h>

struct parc_diffie_hellman_keyshare;
typedef struct parc_diffie_hellman_keyshare PARCDiffieHellmanKeyShare;

/**
 * Create a `PARCDiffieHellmanKeyShare` instance to hold one public and private
 * Diffie Hellman key share for the specified group.
 *
 * @param [in] groupType A type of PARCDiffieHellmanGroup
 *
 * @return NULL Memory could not be allocated.
 * @return non-NULL A pointer to a `PARCDiffieHellmanKeyShare` instance.
 *
 * Example:
 * @code
 * {
 *     PARCDiffieHellmanKeyShare *keyShare = parcDiffieHellmanKeyShare_Create(PARCDiffieHellmanGroup_Secp521r1);
 *
 *     parcDiffieHellmanKeyShare_Release(&keyShare);
 * }
 * @endcode
 */
PARCDiffieHellmanKeyShare *parcDiffieHellmanKeyShare_Create(PARCDiffieHellmanGroup groupType);

/**
 * Increase the number of references to an instance of this object.
 *
 * Note that new instance is not created, only that the given instance's reference count
 * is incremented. Discard the reference by invoking `parcDiffieHellman_Release()`.
 *
 * @param [in] keyShare A `PARCDiffieHellmanKeyShare` instance.
 *
 * @return The value of the input parameter @p instance.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     PARCDiffieHellmanKeyShare *keyShare = parcDiffieHellmanKeyShare_Acquire(keyShareInstance);
 *
 *     parcDiffieHellmanKeyShare_Release(&keyShare);
 * }
 * @endcode
 *
 * @see parcDiffieHellmanKeyShare_Release
 */
PARCDiffieHellmanKeyShare *parcDiffieHellmanKeyShare_Acquire(const PARCDiffieHellmanKeyShare *keyShare);

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
 * @param [in,out] keyShareP A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     ...
 *
 *     PARCDiffieHellmanKeyShare *keyShare = parcDiffieHellmanKeyShare_Acquire(keyShareInstance);
 *
 *     parcDiffieHellmanKeyShare_Release(&keyShare);
 * }
 * @endcode
 */
void parcDiffieHellmanKeyShare_Release(PARCDiffieHellmanKeyShare **keyShareP);

/**
 * Serialize the public key part of a `PARCDiffieHellmanKeyShare.`
 *
 * The public key is saved to a `PARCBuffer` and can be used for transport if needed.
 *
 * @param [in] keyShare A `PARCDiffieHellmanKeyShare` instance.
 *
 * @return A `PARCBuffer` containing the public key of this key share.
 *
 * Example:
 * @code
 * {
 *     PARCDiffieHellmanKeyShare *keyShare = parcDiffieHellmanKeyShare_Create(PARCDiffieHellmanGroup_Secp521r1);
 *
 *     PARCBuffer *publicKey = parcDiffieHellmanKeyShare_SerializePublicKey(keyShare);
 *     // use the public key
 *
 *     parcBuffer_Release(&publicKey);
 *     parcDiffieHellmanKeyShare_Release(&keyShare);
 * }
 * @endcode
 */
PARCBuffer *parcDiffieHellmanKeyShare_SerializePublicKey(PARCDiffieHellmanKeyShare *keyShare);

/**
 * Combine a `PARCDiffieHellmanKeyShare` with an encoded public key to create a shared secret.
 *
 * @param [in] keyShare A `PARCDiffieHellmanKeyShare` instance.
 * @param [in] publicShare The public key share to use to derive the shared secrect.
 *
 * @return A `PARCBuffer` containing the shared secret from the Diffie Hellman exchange.
 *
 * Example:
 * @code
 * {
 *     PARCDiffieHellmanKeyShare *keyShare = parcDiffieHellmanKeyShare_Create(PARCDiffieHellmanGroup_Secp521r1);
 *
 *     PARCBuffer *publicKey = parcDiffieHellmanKeyShare_SerializePublicKey(keyShare);
 *
 *     ...
 *
 *     PARCBuffer *sharedSecret = parcDiffieHellmanKeyShare_Combine(keyShare, publicKey);
 *     // use the shared secret to derive other cryptographic secrets.
 *
 *     parcBuffer_Release(&sharedSecret);
 *     parcBuffer_Release(&publicKey);
 *     parcDiffieHellmanKeyShare_Release(&keyShare);
 * }
 * @endcode
 */
PARCBuffer *parcDiffieHellmanKeyShare_Combine(PARCDiffieHellmanKeyShare *keyShare, PARCBuffer *publicShare);
#endif // libparc_parc_DiffieHellmanKeyShare_h
