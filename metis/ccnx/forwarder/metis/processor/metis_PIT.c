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
 * Generic interface to PIT table
 *
 */

#include <config.h>
#include <stdio.h>
#include <LongBow/runtime.h>

#include <ccnx/forwarder/metis/processor/metis_PIT.h>

void *
metisPIT_Closure(const MetisPIT *pit)
{
    return pit->closure;
}

void
metisPIT_Release(MetisPIT **pitPtr)
{
    (*pitPtr)->release(pitPtr);
}

MetisPITVerdict
metisPIT_ReceiveInterest(MetisPIT *pit, MetisMessage *interestMessage)
{
    return pit->receiveInterest(pit, interestMessage);
}

MetisNumberSet *
metisPIT_SatisfyInterest(MetisPIT *pit, const MetisMessage *objectMessage)
{
    return pit->satisfyInterest(pit, objectMessage);
}

void
metisPIT_RemoveInterest(MetisPIT *pit, const MetisMessage *interestMessage)
{
    pit->removeInterest(pit, interestMessage);
}

MetisPitEntry *
metisPIT_GetPitEntry(const MetisPIT *pit, const MetisMessage *interestMessage)
{
    return pit->getPitEntry(pit, interestMessage);
}
