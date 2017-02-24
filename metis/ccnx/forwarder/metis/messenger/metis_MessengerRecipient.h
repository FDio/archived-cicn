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
 * @file metis_MessengerRecipient.h
 * @brief A recipient represents the entity that will recieve a Missive from the Messenger.
 *
 * A recipient is identified by the pair (contenxt, callback).  The context is the recipients
 * context, such as it's object pointer.  The callback is the function the recipient uses
 * to receive a Missive.
 *
 * If the receiver is going to do a lot of work or potentially send other missives, the receiver
 * should queue the received notifications and process them in its own slice.
 *
 * A recipient will receive a reference counted copy of the missive, so it must call
 * {@link metisMissive_Release} on it.
 *
 *
 */

#ifndef Metis_metis_MessengerRecipient_h
#define Metis_metis_MessengerRecipient_h

struct metis_messenger_recipient;
typedef struct metis_messenger_recipient MetisMessengerRecipient;

/**
 * @typedef MetisMessengerRecipientCallback
 * @abstract A recipient implements a callback to receive Missives.
 * @constant recipient The recipient to recieve the missive
 * @constant missive The missive, recipient must call {@link metisMissive_Release} on it
 * @discussion <#Discussion#>
 */
typedef void (MetisMessengerRecipientCallback)(MetisMessengerRecipient *recipient, MetisMissive *missive);

/**
 * Creates a Recipient, which represents a reciever of missives.
 *
 * Creates a Recipient that can be registerd with the Messenger using {@link metisMessenger_Register}.
 *
 * @param [in] recipientContext This pointer will be passed back to the recipient with each missive, may be NULL
 * @param [in] recipientCallback The function that receives the missive, must be non-NULL.
 *
 * @return non-null A recipient object
 *
 * Example:
 * @code
 * @endcode
 */
MetisMessengerRecipient *metisMessengerRecipient_Create(void *recipientContext, MetisMessengerRecipientCallback *recipientCallback);

/**
 * Destroys a recipient.  You should unregister it first.
 *
 * Destroying a recipient does not unregister it, so be sure to call
 * {@link metisMessenger_Unregister} first.
 *
 * @param [in,out] recipientPtr Double pointer to the recipient to destroy, will be NULL'd.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisMessengerRecipient_Destroy(MetisMessengerRecipient **recipientPtr);

/**
 * Returns the recipient context passed on Create
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in] recipient The recipient object
 *
 * @return pointer The context pointer used to create the object, maybe NULL
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void *metisMessengerRecipient_GetRecipientContext(MetisMessengerRecipient *recipient);

/**
 * Delivers a Missive to the recipient
 *
 * Passes the missive to the recipients callback.
 *
 * A recipient will receive a reference counted copy of the missive, so it must call
 * {@link metisMissive_Release} on it.
 *
 * @param [in] recipient The receiver
 * @param [in] missive The message to send
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisMessengerRecipient_Deliver(MetisMessengerRecipient *recipient, MetisMissive *missive);
#endif // Metis_metis_MessengerRecipient_h
