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
 * @file parc_Security.h
 * @ingroup security
 * @brief PARC Security Library framework director.
 *
 */
#ifndef libparc_parc_Security_h
#define libparc_parc_Security_h

#include <stdbool.h>

/**
 * Initialise the PARC Security framework.
 *
 * This function may be called multiple times,
 * each time incrementing a reference count that is decremented by calls to `parcSecurityFini`.
 *
 * Example:
 * @code
 * {
 *     parcSecurity_Init();
 * }
 * @endcode
 *
 * @see parcSecurity_Fini
 */
void parcSecurity_Init(void);

/**
 * Deinitialise the PARC Security framework.
 *
 * Example:
 * @code
 * {
 *     parcSecurity_Fini();
 * }
 * @endcode
 *
 * @see parcSecurity_Init
 */
void parcSecurity_Fini(void);

/**
 * Assert that the PARC Security Framework is initalised.
 *
 * Example:
 * @code
 * {
 *     parcSecurity_AssertIsInitialized();
 *
 * }
 * @endcode
 *
 * @see parcSecurity_Init
 */
void parcSecurity_AssertIsInitialized(void);

/**
 * Determine if the PARC Security Framework is initalised.
 *
 * @return True if the PARC Security Framework is initalised.
 * @return False otherwise
 *
 * Example:
 * @code
 * {
 *     parcSecurity_IsInitialized();
 * }
 * @endcode
 *
 * @see parcSecurity_Init
 */
bool parcSecurity_IsInitialized(void);
#endif // libparc_parc_Security_h
