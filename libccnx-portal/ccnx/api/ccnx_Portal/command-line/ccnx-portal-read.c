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

#include <stdio.h>

#include <LongBow/runtime.h>

#include <ccnx/api/ccnx_Portal/ccnx_Portal.h>

#include <parc/security/parc_Security.h>
#include <parc/security/parc_PublicKeySignerPkcs12Store.h>
#include <parc/security/parc_IdentityFile.h>

#include <ccnx/common/ccnx_Name.h>
#include <ccnx/common/ccnx_SignedInfo.h>
#include <ccnx/common/ccnx_ContentObject.h>
#include <ccnx/common/ccnx_TimeStamp.h>

PARCIdentity *
getIdentity_FromFile(const char *keystoreFileName, const char *password)
{
    PARCIdentity *result = NULL;

    PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreFileName, password);
    if (identityFile != NULL) {
        result = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);
        parcIdentityFile_Release(&identityFile);
    }

    return result;
}

int
reader_writer(CCNxPortalFactory *factory, const char *uri)
{
    CCNxPortal *portal = ccnxPortalFactory_GetInstance(factory, ccnxPortalTypeDatagram, ccnxPortalProtocol_TLV, &ccnxPortalAttributes_Blocking);

    CCNxName *prefix = ccnxName_CreateFromURI(uri);
    CCNxName *bye = ccnxName_CreateFromURI("lci:/Hello/Goodbye%21");
    CCNxName *contentname = ccnxName_CreateFromURI("lci:/Hello/World");

    if (ccnxPortal_Listen(portal, prefix, 365 * 86400, CCNxStackTimeout_Never)) {
        CCNxMetaMessage *msg;

        while ((msg = ccnxPortal_Receive(portal, CCNxStackTimeout_Never)) != NULL) {
            if (ccnxMetaMessage_IsInterest(msg)) {
                CCNxInterest *interest = ccnxMetaMessage_GetInterest(msg);

                CCNxName *interestName = ccnxInterest_GetName(interest);

                if (ccnxName_Equals(interestName, contentname)) {
                    const PARCKeyId *publisherKeyId = ccnxPortal_GetKeyId(portal);

                    char buffer[128];
                    time_t theTime = time(0);
                    sprintf(buffer, "Hello World. The time is %s", ctime(&theTime));

                    PARCBuffer *payload = parcBuffer_CreateFromArray(buffer, 128);
                    parcBuffer_Flip(payload);

                    PARCBuffer *b = parcBuffer_Acquire(payload);
                    CCNxContentObject *uob = ccnxContentObject_CreateWithNameAndPayload(contentname, b);

                    // missing NULL check, case 1024

                    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromContentObject(uob);
                    if (ccnxPortal_Send(portal, message, CCNxTransportStackTimeCCNxStackTimeout_Neverout_Never) == false) {
                        fprintf(stderr, "ccnx_write failed\n");
                    }
                    // ccnxMessage_Release(message);
                } else if (ccnxName_Equals(interestName, bye)) {
                    break;
                }
            } else {
                ccnxMetaMessage_Display(msg, 0);
            }
            ccnxMetaMessage_Release(&msg);
        }
    }

    ccnxName_Release(&prefix);
    ccnxName_Release(&bye);
    ccnxName_Release(&contentname);

    ccnxPortal_Release(&portal);

    return 0;
}

int
ccnx_Portal_Reader(char *keystoreFileName, const char *password, const char *uri)
{
    parcSecurity_Init();

    PARCIdentity *identity = getIdentity_FromFile(keystoreFileName, password);

    if (identity != NULL) {
        CCNxPortalFactory *factory = ccnxPortalFactory_Create(identity);

        reader_writer(factory, uri);

        ccnxPortalFactory_Release(&factory);
        parcIdentity_Release(&identity);
    }

    parcSecurity_Fini();

    return 0;
}

int
main(int argc, char *argv[argc])
{
    char *keystoreFileName = argv[1];
    char *password = argv[2];
    char *uri = argv[3];

    keystoreFileName = "/tmp/keystore";
    password = "password";
    uri = "lci:/Hello";

    // read fileName password  lci://my/name  lci
    return ccnx_Portal_Reader(keystoreFileName, password, uri);
}
