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

#include "iPing_Common.h"

#include <parc/security/parc_Security.h>
#include <parc/security/parc_Pkcs12KeyStore.h>
#include <parc/security/parc_IdentityFile.h>

const size_t ccnxPing_DefaultReceiveTimeoutInUs = 1000000; // 1 second
const size_t ccnxPing_DefaultPayloadSize = 1400;
const size_t mediumNumberOfPings = 100;
const size_t smallNumberOfPings = 10;

static PARCIdentity *_ccnxPingCommon_CreateAndGetIdentity(const char *keystoreName,
                                                          const char *keystorePassword,
                                                          const char *subjectName) {
  parcSecurity_Init();

  unsigned int keyLength = 1024;
  unsigned int validityDays = 30;

  bool success = parcPkcs12KeyStore_CreateFile(keystoreName, keystorePassword, subjectName, keyLength, validityDays);
  assertTrue(success,
             "parcPkcs12KeyStore_CreateFile('%s', '%s', '%s', %d, %d) failed.",
             keystoreName,
             keystorePassword,
             subjectName,
             keyLength,
             validityDays);

  PARCIdentityFile *identityFile = parcIdentityFile_Create(keystoreName, keystorePassword);
  PARCIdentity *result = parcIdentity_Create(identityFile, PARCIdentityFileAsPARCIdentity);
  parcIdentityFile_Release(&identityFile);

  parcSecurity_Fini();

  return result;
}

CCNxPortalFactory *ccnxPingCommon_SetupPortalFactory(const char *keystoreName,
                                                     const char *keystorePassword,
                                                     const char *subjectName) {
  PARCIdentity *identity = _ccnxPingCommon_CreateAndGetIdentity(keystoreName, keystorePassword, subjectName);
  CCNxPortalFactory *result = ccnxPortalFactory_Create(identity);
  parcIdentity_Release(&identity);

  return result;
}
