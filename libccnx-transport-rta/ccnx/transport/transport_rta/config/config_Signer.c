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

//
//  config_Signer.c
//  Libccnx
//
//

#include <config.h>
#include <stdio.h>
#include <string.h>
#include "config_Signer.h"
#include "config_PublicKeySigner.h"
#include "config_SymmetricKeySigner.h"

#include <ccnx/transport/transport_rta/core/components.h>

#include <LongBow/runtime.h>

static const char param_SIGNER[] = "SIGNER";

const char *
signer_GetName()
{
    return param_SIGNER;
}

/**
 * Determine which signer is configured
 */
SignerType
signer_GetImplementationType(PARCJSON *connectionJson)
{
    assertNotNull(connectionJson, "Parameter must be non-null");

    PARCJSONValue *value = parcJSON_GetValueByName(connectionJson, signer_GetName());
    assertNotNull(value, "signer must be specified in the connection configuration");
    PARCBuffer *sBuf = parcJSONValue_GetString(value);
    const char *signer_name = parcBuffer_Overlay(sBuf, 0);

    assertNotNull(signer_name, "Name of signer must be non-null in connection configuration");

    if (strcasecmp(signer_name, publicKeySigner_GetName()) == 0) {
        return SignerType_PublicKeySigner;
    }

    if (strcasecmp(signer_name, symmetricKeySigner_GetName()) == 0) {
        return SignerType_SymmetricKeySigner;
    }

    return SignerType_Unknown;
}
