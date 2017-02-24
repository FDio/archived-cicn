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

#include <ccnx/forwarder/metis/messenger/metis_Messenger.h>
#include <ccnx/forwarder/metis/messenger/metis_MessengerRecipient.h>

struct metis_messenger_recipient {
    void *context;
    MetisMessengerRecipientCallback *notify;
};

MetisMessengerRecipient *
metisMessengerRecipient_Create(void *recipientContext, MetisMessengerRecipientCallback *recipientCallback)
{
    assertNotNull(recipientCallback, "Parameter recipientCallback must be non-null");

    MetisMessengerRecipient *recipient = parcMemory_AllocateAndClear(sizeof(MetisMessengerRecipient));
    assertNotNull(recipient, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisMessengerRecipient));
    recipient->context = recipientContext;
    recipient->notify = recipientCallback;
    return recipient;
}

void
metisMessengerRecipient_Destroy(MetisMessengerRecipient **recipientPtr)
{
    assertNotNull(recipientPtr, "Parameter must be non-null double pointer");
    assertNotNull(*recipientPtr, "Parameter must dereference to non-null pointer");

    parcMemory_Deallocate((void **) recipientPtr);
    *recipientPtr = NULL;
}

void *
metisMessengerRecipient_GetRecipientContext(MetisMessengerRecipient *recipient)
{
    assertNotNull(recipient, "Parameter must be non-null");

    return recipient->context;
}

void
metisMessengerRecipient_Deliver(MetisMessengerRecipient *recipient, MetisMissive *missive)
{
    assertNotNull(recipient, "Parameter must be non-null");
    recipient->notify(recipient, missive);
}
