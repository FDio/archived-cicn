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


#include <stdint.h>
#include <stdio.h>
#include <ccnx/forwarder/metis/core/metis_Connection.h>
#include <ccnx/forwarder/metis/core/metis_Wldr.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <parc/logging/parc_LogReporterTextStdout.h>

struct metis_wldr_buffer {
    MetisMessage *message;
    uint8_t rtx_counter;
};

typedef struct metis_wldr_buffer MetisWldrBuffer;

struct metis_wldr_state {
    uint16_t expected_label;
    uint16_t next_label;
    MetisWldrBuffer *buffer[BUFFER_SIZE];
};

MetisWldr *
metisWldr_Init()
{
    MetisWldr *wldr = parcMemory_AllocateAndClear(sizeof(MetisWldr));
    assertNotNull(wldr, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisWldr));
    wldr->expected_label = 1;
    wldr->next_label = 1;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        MetisWldrBuffer *entry = parcMemory_AllocateAndClear(sizeof(MetisWldrBuffer));
        assertNotNull(entry, "WldrBuffer init: parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisWldrBuffer));
        entry->message = NULL;
        entry->rtx_counter = 0;
        wldr->buffer[i] = entry;
    }
    return wldr;
}

void
metisWldr_ResetState(MetisWldr *wldr)
{
    wldr->expected_label = 1;
    wldr->next_label = 1;
    for (int i = 0; i < BUFFER_SIZE; i++) {
        wldr->buffer[i]->message = NULL;
        wldr->buffer[i]->rtx_counter = 0;
    }
}

void
metisWldr_Destroy(MetisWldr **wldrPtr)
{
    MetisWldr *wldr = *wldrPtr;
    for (unsigned i = 0; i < BUFFER_SIZE; i++) {
        if (wldr->buffer[i]->message != NULL) {
            metisMessage_Release(&(wldr->buffer[i]->message));
            parcMemory_Deallocate((void **) &(wldr->buffer[i]));
        }
    }
    parcMemory_Deallocate((void **) &wldr);
    *wldrPtr = NULL;
}


static void
_metisWldr_RetransmitPacket(MetisWldr *wldr, const MetisConnection *conn, uint16_t label)
{
    if (wldr->buffer[label % BUFFER_SIZE]->message == NULL) {
        printf("the required message for retransmission is not in the buffer\n");
        return;
    }

    if (wldr->buffer[label % BUFFER_SIZE]->rtx_counter < MAX_RTX) {
        MetisMessage *msg = wldr->buffer[label % BUFFER_SIZE]->message;

        //printf("-----retransmit packet label = %d, new label = %d\n", label, wldr->next_label);
        metisMessage_SetWldrLabel(msg, wldr->next_label);

        if (wldr->buffer[wldr->next_label % BUFFER_SIZE]->message != NULL) {
            //printf("-------------release message in retransmit packet, %d %d\n",(wldr->next_label % BUFFER_SIZE),wldr->next_label);
            metisMessage_Release(&(wldr->buffer[wldr->next_label % BUFFER_SIZE]->message));
        }

        wldr->buffer[wldr->next_label % BUFFER_SIZE]->message = msg;
        wldr->buffer[wldr->next_label % BUFFER_SIZE]->rtx_counter = wldr->buffer[label % BUFFER_SIZE]->rtx_counter + 1;
        metisMessage_Acquire(wldr->buffer[wldr->next_label % BUFFER_SIZE]->message);
        wldr->next_label++;
        metisConnection_ReSend(conn, msg);
    }
}

static void
_metisWldr_SendWldrNotificaiton(MetisWldr *wldr, const MetisConnection *conn, MetisMessage *message, uint16_t expected_lbl, uint16_t received_lbl)
{
    //here we create a copy of the last message received and we use it as a loss notification
    //this can be made more efficient using a pre-encoded message with a small size
    MetisMessage *notification = metisMessage_Slice(message, 0, metisMessage_Length(message), 0, NULL);
    metisMessage_SetWldrNotification(notification, expected_lbl, received_lbl);
    //printf("------------send notification %u, %u\n",expected_lbl, received_lbl);
    assertNotNull(notification, "Got null from Slice");
    metisConnection_ReSend(conn, notification);
}


void
metisWldr_SetLabel(MetisWldr *wldr, MetisMessage *message)
{
    //we send the packet for the first time
    metisMessage_SetWldrLabel(message, wldr->next_label);
    if (wldr->buffer[wldr->next_label % BUFFER_SIZE]->message != NULL) {
        //printf("-------------release message in set label packet, %d %d\n",(wldr->next_label % BUFFER_SIZE),wldr->next_label);
        metisMessage_Release(&(wldr->buffer[wldr->next_label % BUFFER_SIZE]->message));
    }
    metisMessage_Acquire(message);
    wldr->buffer[wldr->next_label % BUFFER_SIZE]->message = message;
    wldr->buffer[wldr->next_label % BUFFER_SIZE]->rtx_counter = 0;
    wldr->next_label++;
}

void
metisWldr_DetectLosses(MetisWldr *wldr, const MetisConnection *conn, MetisMessage *message)
{
    if (metisMessage_HasWldr(message)) {
        uint8_t wldr_type = (uint8_t) metisMessage_GetWldrType(message);
        if (wldr_type == WLDR_LBL) {
            uint16_t pkt_lbl = (uint16_t) metisMessage_GetWldrLabel(message);
            //printf("--------------received packet label %u\n", pkt_lbl);
            if (pkt_lbl != wldr->expected_label) {
                //if the received packet label is 1 and the expected packet label > pkt_lbl
                //usually we are in the case where a remove note disconnected for a while
                //and reconnected on this same connection, so the two nodes are out of synch
                //for this reason we do not send any notification, we just synch the labels

                if ((pkt_lbl != 1) || (wldr->expected_label < pkt_lbl)) {
                    _metisWldr_SendWldrNotificaiton(wldr, conn, message, wldr->expected_label, pkt_lbl);
                }

                //here we always synch
                wldr->expected_label = (uint16_t) (pkt_lbl + 1);
            } else {
                wldr->expected_label++;
            }
        } else if (wldr_type == WLDR_NOTIFICATION) {
            uint16_t expected_lbl = (uint16_t) metisMessage_GetWldrLabel(message);
            uint16_t received_lbl = (uint16_t) metisMessage_GetWldrLastReceived(message);
            //printf("------------received notification %u, %u\n",expected_lbl, received_lbl);
            if ((wldr->next_label - expected_lbl) > BUFFER_SIZE) {
                //printf("--------------the packets are not in the buffer anymore %u %u %u\n", wldr->next_label,
                //       expected_lbl, BUFFER_SIZE);
                //the packets are not in the buffer anymore
                return;
            }
            while (expected_lbl < received_lbl) {
                _metisWldr_RetransmitPacket(wldr, conn, expected_lbl);
                expected_lbl++;
            }
        }
    }
}
