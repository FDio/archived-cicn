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

#include <LongBow/runtime.h>

#include <parc/algol/parc_Object.h>
#include <parc/security/parc_Identity.h>
#include <parc/algol/parc_DisplayIndented.h>

struct parc_identity {
    void *instance;
    const PARCIdentityInterface *interface;
};

static void
_parcIdentity_Destroy(PARCIdentity **identityPtr)
{
    PARCIdentity *identity = *identityPtr;

    identity->interface->Release(&identity->instance);
}

parcObject_ExtendPARCObject(PARCIdentity, _parcIdentity_Destroy, NULL, NULL, NULL, NULL, NULL, NULL);

bool
parcIdentity_IsValid(const PARCIdentity *identity)
{
    bool result = false;

    if (identity != NULL) {
        result = true;
    }

    return result;
}

void
parcIdentity_AssertValid(const PARCIdentity *identity)
{
    trapInvalidValueIf(parcIdentity_IsValid(identity) == false, "PARCIdentity");
}

PARCIdentity *
parcIdentity_Create(PARCObject *instance, const PARCIdentityInterface *interface)
{
    assertNotNull(interface, "Got null interface in parcIdentity_Create");

    PARCIdentity *result = parcObject_CreateInstance(PARCIdentity);

    result->instance = parcObject_Acquire(instance);
    result->interface = interface;

    return result;
}

parcObject_ImplementAcquire(parcIdentity, PARCIdentity);

parcObject_ImplementRelease(parcIdentity, PARCIdentity);

bool
parcIdentity_Equals(const PARCIdentity *a, const PARCIdentity *b)
{
    if (a == b) {
        return true;
    }
    if (a == NULL || b == NULL) {
        return false;
    }
    return a->interface->Equals(a->instance, b->instance);
}


const char *
parcIdentity_GetFileName(const PARCIdentity *identity)
{
    return identity->interface->GetFileName(identity->instance);
}

const char *
parcIdentity_GetPassWord(const PARCIdentity *identity)
{
    return identity->interface->GetPassWord(identity->instance);
}

PARCSigner *
parcIdentity_CreateSigner(const PARCIdentity *identity, PARCCryptoHashType hash)
{
  return identity->interface->GetSigner(identity->instance, hash);
}

void
parcIdentity_Display(const PARCIdentity *identity, int indentation)
{
    assertNotNull(identity->interface->Display, "Got null implementation in parcIdentity_Display");

    parcDisplayIndented_PrintLine(indentation, "PARCIdentity@%p {", identity);
    parcDisplayIndented_PrintLine(indentation, ".instance=");
    identity->interface->Display(identity->instance, 1);
    parcDisplayIndented_PrintLine(indentation, "}\n");
}
