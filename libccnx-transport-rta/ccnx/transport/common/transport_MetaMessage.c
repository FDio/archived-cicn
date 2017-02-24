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

#include <LongBow/runtime.h>

#include <ccnx/transport/common/transport_MetaMessage.h>
#include <ccnx/common/codec/ccnxCodec_TlvPacket.h>
#include <ccnx/common/ccnx_WireFormatMessage.h>
#include <ccnx/common/ccnx_Manifest.h>

CCNxMetaMessage *
ccnxMetaMessage_CreateFromInterest(const CCNxInterest *interest)
{
    return ccnxMetaMessage_Acquire((CCNxMetaMessage *) interest);
}

CCNxMetaMessage *
ccnxMetaMessage_CreateFromContentObject(const CCNxContentObject *contentObject)
{
    return ccnxMetaMessage_Acquire((CCNxMetaMessage *) contentObject);
}

CCNxMetaMessage *
ccnxMetaMessage_CreateFromControl(const CCNxControl *control)
{
    return ccnxMetaMessage_Acquire((CCNxMetaMessage *) control);
}

CCNxMetaMessage *
ccnxMetaMessage_CreateFromManifest(const CCNxManifest *manifest)
{
    return ccnxMetaMessage_Acquire((CCNxMetaMessage *) manifest);
}


CCNxContentObject *
ccnxMetaMessage_GetContentObject(const CCNxMetaMessage *message)
{
    return (CCNxContentObject *) message;
}

CCNxInterest *
ccnxMetaMessage_GetInterest(const CCNxMetaMessage *message)
{
    return (CCNxInterest *) message;
}

CCNxInterestReturn *
ccnxMetaMessage_GetInterestReturn(const CCNxMetaMessage *message)
{
    return (CCNxInterestReturn *) message;
}

CCNxControl *
ccnxMetaMessage_GetControl(const CCNxMetaMessage *message)
{
    return (CCNxControl *) message;
}

CCNxManifest *
ccnxMetaMessage_GetManifest(const CCNxMetaMessage *message)
{
    return (CCNxManifest *) message;
}

CCNxMetaMessage *
ccnxMetaMessage_Acquire(const CCNxMetaMessage *message)
{
    return ccnxTlvDictionary_Acquire(message);
}

void
ccnxMetaMessage_Release(CCNxMetaMessage **messagePtr)
{
    ccnxTlvDictionary_Release(messagePtr);
}

void
ccnxMetaMessage_Display(const CCNxMetaMessage *message, int indentation)
{
    ccnxTlvDictionary_Display(message, indentation);
}

bool
ccnxMetaMessage_IsContentObject(const CCNxMetaMessage *message)
{
    return ccnxTlvDictionary_IsContentObject(message);
}

bool
ccnxMetaMessage_IsInterest(const CCNxMetaMessage *message)
{
    return ccnxTlvDictionary_IsInterest(message);
}

bool
ccnxMetaMessage_IsInterestReturn(const CCNxMetaMessage *message)
{
    return ccnxTlvDictionary_IsInterestReturn(message);
}

bool
ccnxMetaMessage_IsControl(const CCNxMetaMessage *message)
{
    return ccnxTlvDictionary_IsControl(message);
}

bool
ccnxMetaMessage_IsManifest(const CCNxMetaMessage *message)
{
    return ccnxTlvDictionary_IsManifest(message);
}

/**
 * Given an iovec encoded version of a TlvDictionary, which is what we get when we call ccnxCodecTlvPacket_DictionaryEncode(),
 * linearize it into a PARCBuffer so we can treat it as a PARCBuffer.
 */
static PARCBuffer *
_iovecToParcBuffer(const CCNxCodecNetworkBufferIoVec *iovec)
{
    PARCBuffer *result = NULL;

    size_t iovcnt = ccnxCodecNetworkBufferIoVec_GetCount((CCNxCodecNetworkBufferIoVec *) iovec);
    const struct iovec *array = ccnxCodecNetworkBufferIoVec_GetArray((CCNxCodecNetworkBufferIoVec *) iovec);

    size_t totalbytes = 0;
    for (int i = 0; i < iovcnt; i++) {
        totalbytes += array[i].iov_len;
    }

    result = parcBuffer_Allocate(totalbytes);
    for (int i = 0; i < iovcnt; i++) {
        parcBuffer_PutArray(result, array[i].iov_len, array[i].iov_base);
    }

    parcBuffer_Flip(result);


    return result;
}

CCNxMetaMessage *
ccnxMetaMessage_CreateFromWireFormatBuffer(PARCBuffer *rawMessage)
{
    CCNxMetaMessage *result = NULL;

    CCNxWireFormatMessage *message = ccnxWireFormatMessage_Create(rawMessage);

    if (message != NULL) {
        // Get the dictionary from the ccnxWireFormatMessage.
        CCNxTlvDictionary *dictionary = ccnxWireFormatMessage_GetDictionary(message);

        // We have a partially unpacked dictionary now, but we need to more fully unpack it for our processing.
        bool success = ccnxCodecTlvPacket_BufferDecode(rawMessage, dictionary);

        if (success) {
            result = (CCNxMetaMessage *) dictionary;
        } else {
            ccnxWireFormatMessage_Release(&message);
            result = NULL;
        }
    }

    return result;
}

PARCBuffer *
ccnxMetaMessage_CreateWireFormatBuffer(CCNxMetaMessage *message, PARCSigner *signer)
{
    CCNxCodecNetworkBufferIoVec *iovec = ccnxCodecTlvPacket_DictionaryEncode(message, signer);

    // iovec has the wireformat version of 'interest' now.

    PARCBuffer *result = _iovecToParcBuffer(iovec);

    ccnxCodecNetworkBufferIoVec_Release(&iovec);

    return result;
}
