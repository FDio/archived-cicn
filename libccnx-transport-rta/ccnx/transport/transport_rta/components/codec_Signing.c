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

#include <config.h>
#include <stdio.h>

#include <LongBow/runtime.h>

#include <parc/algol/parc_Memory.h>

#include <parc/security/parc_PublicKeySigner.h>
#include <parc/security/parc_Pkcs12KeyStore.h>
#include <parc/security/parc_SymmetricKeySigner.h>
#include <parc/security/parc_SymmetricKeyStore.h>
#include <parc/security/parc_KeyStore.h>
#include <parc/security/parc_Signer.h>
#include <parc/security/parc_CryptoHashType.h>

#include <ccnx/transport/transport_rta/config/config_Signer.h>
#include "codec_Signing.h"

PARCSigner *
component_Codec_GetSigner(RtaConnection *conn)
{
    PARCSigner *signer = NULL;

    SignerType signertype = signer_GetImplementationType(rtaConnection_GetParameters(conn));

    switch (signertype) {
        case SignerType_SymmetricKeySigner: {
            struct symmetrickeysigner_params params;
            bool success = symmetricKeySigner_GetConnectionParams(rtaConnection_GetParameters(conn), &params);
            assertTrue(success, "Could not retrieve symmetricKeySigner_GetConnectionParams");

            PARCSymmetricKeyStore *symmetricKeyStore = parcSymmetricKeyStore_OpenFile(params.filename, params.password, PARCCryptoHashType_SHA256);
            PARCSymmetricKeySigner *symmetricKeySigner = parcSymmetricKeySigner_Create(symmetricKeyStore, PARCCryptoHashType_SHA256);
            parcSymmetricKeyStore_Release(&symmetricKeyStore);

            signer = parcSigner_Create(symmetricKeySigner, PARCSymmetricKeySignerAsSigner);
            parcSymmetricKeySigner_Release(&symmetricKeySigner);
            assertNotNull(signer, "got null opening FileKeystore '%s'\n", params.filename);
            break;
        }

        case SignerType_PublicKeySigner: {
            struct publickeysigner_params params;
            bool success = publicKeySigner_GetConnectionParams(rtaConnection_GetParameters(conn), &params);
            assertTrue(success, "Could not retrieve publicKeySigner_GetConnectionParams");

            PARCPkcs12KeyStore *pkcs12KeyStore = parcPkcs12KeyStore_Open(params.filename, params.password, PARCCryptoHashType_SHA256);
            PARCKeyStore *keyStore = parcKeyStore_Create(pkcs12KeyStore, PARCPkcs12KeyStoreAsKeyStore);
            parcPkcs12KeyStore_Release(&pkcs12KeyStore);
            PARCPublicKeySigner *publicKeySigner = parcPublicKeySigner_Create(keyStore, PARCCryptoSuite_RSA_SHA256);
            parcKeyStore_Release(&keyStore);

            signer = parcSigner_Create(publicKeySigner, PARCPublicKeySignerAsSigner);
            parcPublicKeySigner_Release(&publicKeySigner);
            assertNotNull(signer, "got null opening FileKeystore '%s'\n", params.filename);
            break;
        }

        default:
            assertTrue(0, "Unsupported signer type %d", signertype);
    }

    assertNotNull(signer, "Did not match a known signer");
    return signer;
}
