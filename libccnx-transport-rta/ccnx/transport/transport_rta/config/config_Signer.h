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
 * @file config_Signer.h
 * @brief Queries the configuration to determine which signer is used
 *
 * <#Detailed Description#>
 *
 */
#ifndef Libccnx_config_Signer_h
#define Libccnx_config_Signer_h

#include <ccnx/transport/common/ccnx_TransportConfig.h>
#include "config_SymmetricKeySigner.h"
#include "config_PublicKeySigner.h"

typedef enum {
    SignerType_Unknown,
    SignerType_PublicKeySigner,
    SignerType_SymmetricKeySigner
} SignerType;

/**
 * Determine which signer is configured.  Each specific implementation will emit a line
 * such as { "SIGNER" : "signer_name" }
 */
SignerType signer_GetImplementationType(PARCJSON *connectionJson);

/**
 * Returns the text string for this component
 *
 * Used as the text key to a JSON block.  You do not need to free it.
 *
 * @return non-null A text string unique to this component
 *
 */
const char *signer_GetName(void);
#endif // Libccnx_config_Signer_h
