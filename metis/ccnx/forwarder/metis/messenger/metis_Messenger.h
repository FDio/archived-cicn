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
 * The EventMessenger is the system that messages events between
 * producers and consumers.
 *
 * Events are delivered in a deferred event cycle to avoid event callbacks
 * firing when the event generator is still running.
 */

#ifndef Metis_metis_Messenger_h
#define Metis_metis_Messenger_h

#include <ccnx/forwarder/metis/core/metis_Dispatcher.h>
#include <ccnx/forwarder/metis/messenger/metis_Missive.h>
#include <ccnx/forwarder/metis/messenger/metis_MessengerRecipient.h>

struct metis_messenger;
typedef struct metis_messenger MetisMessenger;

/**
 * @function metisEventmessenger_Create
 * @abstract Creates an event notification system
 * @discussion
 *   Typically there's only one of these managed by metisForwarder.
 *
 * @param dispatcher is the event dispatcher to use to schedule events.
 * @return <#return#>
 */
MetisMessenger *metisMessenger_Create(MetisDispatcher *dispatcher);

/**
 * @function metisEventMessenger_Destroy
 * @abstract Destroys the messenger system, no notification is sent
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisMessenger_Destroy(MetisMessenger **messengerPtr);

/**
 * @function metisEventMessenger_Send
 * @abstract Send an event message, takes ownership of the event memory
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 */
void metisMessenger_Send(MetisMessenger *messenger, MetisMissive *missive);

/**
 * @function metisEventMessenger_Register
 * @abstract Receive all event messages
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisMessenger_Register(MetisMessenger *messenger, const MetisMessengerRecipient *recipient);

/**
 * @function metisEventMessenger_Unregister
 * @abstract Stop receiving event messages
 * @discussion
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return <#return#>
 */
void metisMessenger_Unregister(MetisMessenger *messenger, const MetisMessengerRecipient *recipient);
#endif // Metis_metis_Messenger_h
