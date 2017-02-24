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
//  config_SymmetricKeySigner.c
//  Libccnx
//
//

#include <config.h>
#include <stdio.h>
#include "config_SymmetricKeySigner.h"

#include <ccnx/transport/transport_rta/core/components.h>

#include <LongBow/runtime.h>

static const char name[] = "SymmetricKeySigner";

static const char param_KEYSTORE_NAME[] = "KEYSTORE_NAME";
static const char param_KEYSTORE_PASSWD[] = "KEYSTORE_PASSWD";
static const char param_SIGNER[] = "SIGNER";

/**
 * Generates:
 *
 * { "SIGNER" : "SymmetricKeySigner",
 * "SymmetricKeySigner" : { "filename" : filename, "password" : password }
 * }
 */
CCNxConnectionConfig *
symmetricKeySigner_ConnectionConfig(CCNxConnectionConfig *connConfig, const char *filename, const char *password)
{
    assertNotNull(connConfig, "Parameter connConfig must be non-null");
    assertNotNull(filename, "Parameter filename must be non-null");
    assertNotNull(password, "Parameter password must be non-null");

    PARCJSONValue *signerNameJson = parcJSONValue_CreateFromCString((char *) symmetricKeySigner_GetName());
    ccnxConnectionConfig_Add(connConfig, param_SIGNER, signerNameJson);
    parcJSONValue_Release(&signerNameJson);

    PARCJSON *keystoreJson = parcJSON_Create();
    parcJSON_AddString(keystoreJson, param_KEYSTORE_NAME, filename);
    parcJSON_AddString(keystoreJson, param_KEYSTORE_PASSWD, password);

    PARCJSONValue *value = parcJSONValue_CreateFromJSON(keystoreJson);
    parcJSON_Release(&keystoreJson);

    ccnxConnectionConfig_Add(connConfig, symmetricKeySigner_GetName(), value);
    parcJSONValue_Release(&value);
    return connConfig;
}

const char *
symmetricKeySigner_GetName()
{
    return name;
}

/**
 * If successful, return true and fill in the parameters in the output argument
 */
bool
symmetricKeySigner_GetConnectionParams(PARCJSON *connectionJson, struct symmetrickeysigner_params *output)
{
    assertNotNull(connectionJson, "Parameter connectionJson must be non-null");
    assertNotNull(output, "Parameter output must be non-null");

    PARCJSONValue *value = parcJSON_GetValueByName(connectionJson, symmetricKeySigner_GetName());
    assertNotNull(value, "JSON key %s missing", symmetricKeySigner_GetName());
    PARCJSON *keystoreJson = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(keystoreJson, param_KEYSTORE_NAME);
    assertNotNull(value, "JSON key %s missing inside %s", param_KEYSTORE_NAME, symmetricKeySigner_GetName());
    PARCBuffer *sBuf = parcJSONValue_GetString(value);
    output->filename = parcBuffer_Overlay(sBuf, 0);

    value = parcJSON_GetValueByName(keystoreJson, param_KEYSTORE_PASSWD);
    assertNotNull(value, "JSON key %s missing inside %s", param_KEYSTORE_PASSWD, symmetricKeySigner_GetName());
    sBuf = parcJSONValue_GetString(value);
    output->password = parcBuffer_Overlay(sBuf, 0);
    return true;
}
