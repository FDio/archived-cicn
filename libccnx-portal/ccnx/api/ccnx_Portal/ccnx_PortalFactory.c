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

#include <sys/errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include <ccnx/api/ccnx_Portal/ccnx_PortalFactory.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_List.h>
#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_DisplayIndented.h>

#include <parc/security/parc_Signer.h>
#include <parc/security/parc_Security.h>

const char *CCNxPortalFactory_LocalRouterName = "/localstack/portalFactory/LocalRouterName";
const char *CCNxPortalFactory_LocalForwarder = "/localstack/portalFactory/LocalForwarder";
const char *CCNxPortalFactory_LocalRouterTimeout = "/localstack/portalFactory/LocalRouterTimeout";

struct CCNxPortalFactory {
    const PARCIdentity *identity;
    const PARCSigner *signer;
    const PARCKeyId *keyId;
    const CCNxPortalAttributes *attributeTemplate;
    PARCProperties *properties;
};

static void
_ccnxPortalFactory_Destroy(CCNxPortalFactory **factoryPtr)
{
    CCNxPortalFactory *factory = *factoryPtr;

    parcIdentity_Release((PARCIdentity **) &factory->identity);

    parcSigner_Release((PARCSigner **) &factory->signer);

    parcKeyId_Release((PARCKeyId **) &factory->keyId);

    parcProperties_Release(&factory->properties);

    parcSecurity_Fini();
}

parcObject_ExtendPARCObject(CCNxPortalFactory, _ccnxPortalFactory_Destroy, NULL, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementAcquire(ccnxPortalFactory, CCNxPortalFactory);

parcObject_ImplementRelease(ccnxPortalFactory, CCNxPortalFactory);

CCNxPortalFactory *
ccnxPortalFactory_Create(const PARCIdentity *identity)
{
    parcIdentity_OptionalAssertValid(identity);

    parcSecurity_Init();
    CCNxPortalFactory *result = parcObject_CreateInstance(CCNxPortalFactory);
    if (result != NULL) {
        result->identity = parcIdentity_Acquire(identity);
        result->signer = parcIdentity_CreateSigner(identity);
        result->keyId = parcSigner_CreateKeyId(result->signer);
        result->properties = parcProperties_Create();

        ccnxPortalFactory_SetProperty(result, CCNxPortalFactory_LocalRouterName, "lci:/local/dcr");
        ccnxPortalFactory_SetProperty(result, CCNxPortalFactory_LocalForwarder, "tcp://127.0.0.1:9695");
        ccnxPortalFactory_SetProperty(result, CCNxPortalFactory_LocalRouterTimeout, "1000000");
    }
    return result;
}

const PARCIdentity *
ccnxPortalFactory_GetIdentity(const CCNxPortalFactory *factory)
{
    return factory->identity;
}

const PARCKeyId *
ccnxPortalFactory_GetKeyId(const CCNxPortalFactory *factory)
{
    return factory->keyId;
}

void
ccnxPortalFactory_Display(const CCNxPortalFactory *factory, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "CCNxPortalFactory@%p {", factory);

    parcIdentity_Display(factory->identity, indentation + 1);
    parcProperties_Display(factory->properties, indentation + 1);

    parcDisplayIndented_PrintLine(indentation, "}");
}

CCNxPortal *
ccnxPortalFactory_CreatePortal(const CCNxPortalFactory *factory, CCNxStackImpl *stackImplementation)
{
    return stackImplementation(factory, &ccnxPortalAttributes_NonBlocking);
}

PARCProperties *
ccnxPortalFactory_GetProperties(const CCNxPortalFactory *factory)
{
    return factory->properties;
}

const char *
ccnxPortalFactory_GetProperty(const CCNxPortalFactory *factory, const char *restrict name, const char *restrict defaultValue)
{
    return parcProperties_GetPropertyDefault(factory->properties, name, defaultValue);
}

void
ccnxPortalFactory_SetProperty(const CCNxPortalFactory *factory, const char *restrict name, const char *restrict value)
{
    parcProperties_SetProperty(factory->properties, name, value);
}
