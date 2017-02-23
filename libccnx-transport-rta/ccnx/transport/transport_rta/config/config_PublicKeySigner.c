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
//  config_PublicKeySigner.c
//  Libccnx
//
//
//
#include <config.h>
#include <LongBow/runtime.h>

#include <stdio.h>

#include <parc/security/parc_Identity.h>
#include "config_PublicKeySigner.h"

#include <ccnx/transport/transport_rta/core/components.h>

static const char name[] = "publicKeySigner";

static const char param_KEYSTORE_NAME[] = "KEYSTORE_NAME";
static const char param_KEYSTORE_PASSWD[] = "KEYSTORE_PASSWD";
static const char param_SIGNER[] = "SIGNER";

/**
 * Generates:
 *
 * { "SIGNER" : "publicKeySigner",
 * "publicKeySigner" : { "filename" : filename, "password" : password }
 * }
 */
CCNxConnectionConfig *
configPublicKeySigner_SetIdentity(CCNxConnectionConfig *connConfig, const PARCIdentity *identity)
{
    return publicKeySigner_ConnectionConfig(connConfig,
                                            parcIdentity_GetFileName(identity),
                                            parcIdentity_GetPassWord(identity));
}

CCNxConnectionConfig *
publicKeySigner_ConnectionConfig(CCNxConnectionConfig *connConfig, const char *filename, const char *password)
{
    assertNotNull(connConfig, "Parameter connConfig must be non-null");
    assertNotNull(filename, "Parameter filename must be non-null");
    assertNotNull(password, "Parameter password must be non-null");

    PARCJSONValue *signerNameJson = parcJSONValue_CreateFromCString((char *) publicKeySigner_GetName());
    ccnxConnectionConfig_Add(connConfig, param_SIGNER, signerNameJson);
    parcJSONValue_Release(&signerNameJson);

    PARCJSON *keystoreJson = parcJSON_Create();
    parcJSON_AddString(keystoreJson, param_KEYSTORE_NAME, filename);
    parcJSON_AddString(keystoreJson, param_KEYSTORE_PASSWD, password);

    PARCJSONValue *value = parcJSONValue_CreateFromJSON(keystoreJson);
    parcJSON_Release(&keystoreJson);
    CCNxConnectionConfig *result = ccnxConnectionConfig_Add(connConfig, publicKeySigner_GetName(), value);
    parcJSONValue_Release(&value);
    return result;
}

const char *
publicKeySigner_GetName()
{
    return name;
}

/**
 * If successful, return true and fill in the parameters in the output argument
 */
bool
publicKeySigner_GetConnectionParams(PARCJSON *connectionJson, struct publickeysigner_params *output)
{
    assertNotNull(connectionJson, "Parameter connectionJson must be non-null");
    assertNotNull(output, "Parameter output must be non-null");

    PARCJSONValue *value = parcJSON_GetValueByName(connectionJson, publicKeySigner_GetName());
    assertNotNull(value, "JSON key %s missing", publicKeySigner_GetName());
    PARCJSON *keystoreJson = parcJSONValue_GetJSON(value);

    value = parcJSON_GetValueByName(keystoreJson, param_KEYSTORE_NAME);
    assertNotNull(value, "JSON key %s missing inside %s", param_KEYSTORE_NAME, publicKeySigner_GetName());
    PARCBuffer *sBuf = parcJSONValue_GetString(value);
    output->filename = parcBuffer_Overlay(sBuf, 0);

    value = parcJSON_GetValueByName(keystoreJson, param_KEYSTORE_PASSWD);
    assertNotNull(value, "JSON key %s missing inside %s", param_KEYSTORE_PASSWD, publicKeySigner_GetName());
    sBuf = parcJSONValue_GetString(value);
    output->password = parcBuffer_Overlay(sBuf, 0);


    return true;
}
