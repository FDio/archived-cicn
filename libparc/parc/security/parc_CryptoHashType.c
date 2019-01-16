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
 */
#include <config.h>

#include <stdio.h>
#include <string.h>

#include <parc/assert/parc_Assert.h>

#include <parc/security/parc_CryptoHashType.h>

static struct {
    PARCCryptoHashType type;
    char *name;
} cryptoHashType_ToString[] = {
    { PARCCryptoHashType_SHA256, "PARCCryptoHashType_SHA256" },
    { PARCCryptoHashType_SHA512, "PARCCryptoHashType_SHA512" },
    { PARCCryptoHashType_CRC32C, "PARCCryptoHashType_CRC32C" },
    { 0,                         NULL                        }
};

const char *
parcCryptoHashType_ToString(PARCCryptoHashType type)
{
    for (int i = 0; cryptoHashType_ToString[i].name != NULL; i++) {
        if (cryptoHashType_ToString[i].type == type) {
            return cryptoHashType_ToString[i].name;
        }
    }
    return NULL;
}

PARCCryptoHashType
parcCryptoHashType_FromString(const char *name)
{
    for (int i = 0; cryptoHashType_ToString[i].name != NULL; i++) {
        if (strcmp(cryptoHashType_ToString[i].name, name) == 0) {
            return cryptoHashType_ToString[i].type;
        }
    }
    return PARCCryptoHashType_NULL;
}
