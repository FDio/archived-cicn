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
 * @file parc_SecureRandom.h
 * @brief A cryptographically secure pseudorandom number generator that reads
 * from a secure randomness source on the system, e.g., /dev/urandom.
 *
 */
#ifndef Libparc_parc_SecureRandom
#define Libparc_parc_SecureRandom

#include <stdbool.h>

#include <parc/algol/parc_Object.h>

struct parc_securerandom;
typedef struct parc_securerandom PARCSecureRandom;

/**
 * Increase the number of references to a `PARCSecureRandom` instance.
 *
 * Note that new `PARCSecureRandom` is not created,
 * only that the given `PARCSecureRandom` reference count is incremented.
 * Discard the reference by invoking `parcSecureRandom_Release`.
 *
 * @param [in] instance A pointer to a valid PARCSecureRandom instance.
 *
 * @return The same value as @p instance.
 *
 * Example:
 * @code
 * {
 *     PARCSecureRandom *a = parcSecureRandom_Create();
 *
 *     PARCSecureRandom *b = parcSecureRandom_Acquire();
 *
 *     parcSecureRandom_Release(&a);
 *     parcSecureRandom_Release(&b);
 * }
 * @endcode
 */
PARCSecureRandom *parcSecureRandom_Acquire(const PARCSecureRandom *instance);

/**
 * Create an instance of PARCSecureRandom
 *
 * @return non-NULL A pointer to a valid PARCSecureRandom instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCSecureRandom *rng = parcSecureRandom_Create();
 *
 *     parcSecureRandom_Release(&rng);
 * }
 * @endcode
 */
PARCSecureRandom *parcSecureRandom_Create(void);

/**
 * Create an instance of PARCSecureRandom with a specific seed stored in
 * a `PARCBuffer` instance.
 *
 * @param [in] seed A `PARCBuffer` instance.
 *
 * @return non-NULL A pointer to a valid PARCSecureRandom instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCBuffer *seed = ...
 *     PARCSecureRandom *rng = parcSecureRandom_CreateWithSeed(seed);
 *
 *     parcSecureRandom_Release(&rng);
 * }
 * @endcode
 */
PARCSecureRandom *parcSecureRandom_CreateWithSeed(PARCBuffer *seed);

/**
 * Generate a 32-bit unsigned integer from a `PARCSecureRandom` instance.
 *
 * @param [in] rng A `PARCSecureRandom` instance.
 *
 * @return non-NULL A pointer to a valid PARCSecureRandom instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCSecureRandom *rng = parcSecureRandom_Create();
 *
 *     uint32_t nextWord = parcSecureRandom_Next(rng);
 *     // use the word
 * }
 * @endcode
 */
uint32_t parcSecureRandom_Next(PARCSecureRandom *rng);

/**
 * Fill a `PARCBuffer` instance with random bytes from a `PARCSecureRandom` instance.
 * The resultant `PARCBuffer` will be ready for reading, i.e., one does not need
 * to call `parcBuffer_Flip()` on the result.
 *
 * @param [in] rng A `PARCSecureRandom` instance.
 * @param [in] buffer A `PARCBuffer` instance to fill.
 *
 * @return non-NULL A pointer to a valid PARCSecureRandom instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCSecureRandom *rng = parcSecureRandom_Create();
 *
 *     PARCBuffer *buffer = parcBuffer_Allocate(32);
 *     ssize_t numBytes = parcSecureRandom_NextBytes(rng, buffer);
 *     // use the buffer
 * }
 * @endcode
 */
ssize_t parcSecureRandom_NextBytes(PARCSecureRandom *rng, PARCBuffer *buffer);

/**
 * Determine if an instance of `PARCSecureRandom` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a valid PARCSecureRandom instance.
 *
 * @return true The instance is valid.
 * @return false The instance is not valid.
 *
 * Example:
 * @code
 * {
 *     PARCSecureRandom *rng = parcSecureRandom_Create();
 *
 *     if (parcSecureRandom_IsValid(rng)) {
 *         printf("Instance is valid.\n");
 *     }
 *
 *     parcSecureRandom_Release(&rng);
 * }
 * @endcode
 *
 */
bool parcSecureRandom_IsValid(const PARCSecureRandom *instance);

/**
 * Release a previously acquired reference to the given `PARCSecureRandom` instance,
 * decrementing the reference count for the instance.
 *
 * The pointer to the instance is set to NULL as a side-effect of this function.
 *
 * If the invocation causes the last reference to the instance to be released,
 * the instance is deallocated and the instance's implementation will perform
 * additional cleanup and release other privately held references.
 *
 * @param [in,out] instancePtr A pointer to a pointer to the instance to release.
 *
 * Example:
 * @code
 * {
 *     PARCSecureRandom *rng = parcSecureRandom_Create();
 *
 *     parcSecureRandom_Release(&rng);
 * }
 * @endcode
 */
void parcSecureRandom_Release(PARCSecureRandom **instancePtr);
#endif
