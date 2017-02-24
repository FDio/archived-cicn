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

#include <ccnx/forwarder/metis/content_store/metis_ContentStoreInterface.h>

void
metisContentStoreInterface_Release(MetisContentStoreInterface **storeImplPtr)
{
    (*storeImplPtr)->release(storeImplPtr);
}

bool
metisContentStoreInterface_PutContent(MetisContentStoreInterface *storeImpl, MetisMessage *content, uint64_t currentTimeTicks)
{
    return storeImpl->putContent(storeImpl, content, currentTimeTicks);
}

bool
metisContentStoreInterface_RemoveContent(MetisContentStoreInterface *storeImpl, MetisMessage *content)
{
    return storeImpl->removeContent(storeImpl, content);
}

MetisMessage *
metisContentStoreInterface_MatchInterest(MetisContentStoreInterface *storeImpl, MetisMessage *interest)
{
    return storeImpl->matchInterest(storeImpl, interest);
}

size_t
metisContentStoreInterface_GetObjectCapacity(MetisContentStoreInterface *storeImpl)
{
    return storeImpl->getObjectCapacity(storeImpl);
}

size_t
metisContentStoreInterface_GetObjectCount(MetisContentStoreInterface *storeImpl)
{
    return storeImpl->getObjectCount(storeImpl);
}

void
metisContentStoreInterface_Log(MetisContentStoreInterface *storeImpl)
{
    storeImpl->log(storeImpl);
}

void *
metisContentStoreInterface_GetPrivateData(MetisContentStoreInterface *storeImpl)
{
    return storeImpl->_privateData;
}
