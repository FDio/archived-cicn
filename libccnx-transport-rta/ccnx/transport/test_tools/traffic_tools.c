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
#include <string.h>

#include <LongBow/runtime.h>
#include <parc/algol/parc_Memory.h>

#include "traffic_tools.h"
#include <ccnx/common/ccnx_NameSegmentNumber.h>
#include <ccnx/transport/transport_rta/core/rta_Component.h>

#include <ccnx/common/codec/schema_v1/testdata/v1_interest_nameA.h>

#include <ccnx/common/internal/ccnx_InterestDefault.h>

#include <ccnx/api/control/cpi_ControlFacade.h>

/**
 * @function decode_last_component_as_segment
 * @abstract Returns true is last name component is a segment, and returns the segment
 * @discussion
 *   The outputSegment may be null, in which case this function is just a true/false for the
 *   last path segment being an object segment number.
 *
 * @param outputSegment is an output parameter of the segment number, if returns true.  May be null.
 * @return <#return#>
 */
bool
trafficTools_GetObjectSegmentFromName(CCNxName *name, uint64_t *outputSegment)
{
    assertNotNull(name, "Name must be non-null");
    bool success = false;
    size_t segmentCount = ccnxName_GetSegmentCount(name);
    if (segmentCount > 0) {
        CCNxNameSegment *lastSegment = ccnxName_GetSegment(name, segmentCount - 1);
        if (ccnxNameSegment_GetType(lastSegment) == CCNxNameLabelType_CHUNK) {
            if (outputSegment) {
                *outputSegment = ccnxNameSegmentNumber_Value(lastSegment);
            }
            success = true;
        }
    }
    return success;
}

bool
trafficTools_ReadAndVerifySegment(PARCEventQueue *queue, CCNxName *basename, uint64_t expected, PARCBuffer *expectedPayload)
{
    TransportMessage *test_tm;
    CCNxName         *test_name;
    uint64_t segnum;
    CCNxName         *name_copy;

    test_tm = rtaComponent_GetMessage(queue);
    assertNotNull(test_tm, "got null transport message down the stack, expecting interest\n");

    assertTrue(transportMessage_IsInterest(test_tm),
               "Got wrong transport message pointer, is not an interest");

    CCNxTlvDictionary *interestDictionary = transportMessage_GetDictionary(test_tm);
    test_name = ccnxInterest_GetName(interestDictionary);

    bool success = trafficTools_GetObjectSegmentFromName(test_name, &segnum);
    assertTrue(success, "got error decoding last component as segnum: %s", ccnxName_ToString(test_name));
    assertTrue(expected == segnum, "Got wrong segnum, expected %" PRIu64 ", got %" PRIu64 "\n",
               expected, segnum);

    name_copy = ccnxName_Copy(test_name);
    ccnxName_Trim(name_copy, 1);
    assertTrue(ccnxName_Compare(basename, name_copy) == 0,
               "\nName '%s'\ndid not match\nexpected '%s'\nInterest name '%s'\n\n",
               ccnxName_ToString(name_copy),
               ccnxName_ToString(basename),
               ccnxName_ToString(test_name));

    ccnxName_Release(&name_copy);

    if (expectedPayload != NULL) {
        assertTrue(parcBuffer_Equals(expectedPayload, ccnxInterest_GetPayload(interestDictionary)),
                   "Expected the same Interest payload out as was sent in originally.");
    }

    transportMessage_Destroy(&test_tm);

    return true;
}

CCNxContentObject *
trafficTools_CreateSignedContentObject()
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/hello/dolly");
    PARCBuffer *payload = parcBuffer_WrapCString("hello");

    CCNxContentObject *result = ccnxContentObject_CreateWithNameAndPayload(name, payload);

    PARCBuffer *keyId = parcBuffer_WrapCString("keyhash");
    PARCBuffer *sigbits = parcBuffer_WrapCString("siggybits");

    PARCSignature *signature = parcSignature_Create(PARCSigningAlgorithm_RSA, PARCCryptoHashType_SHA256, sigbits);
    parcBuffer_Release(&sigbits);

    ccnxContentObject_SetSignature(result, keyId, signature, NULL);

    parcBuffer_Release(&payload);
    parcBuffer_Release(&keyId);
    parcSignature_Release(&signature);
    ccnxName_Release(&name);

    return result;
}

CCNxContentObject *
trafficTools_CreateContentObjectWithPayload(PARCBuffer *contents)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/hello/dolly");

    CCNxContentObject *result = ccnxContentObject_CreateWithNameAndPayload(name, contents);

    ccnxName_Release(&name);

    return result;
}

TransportMessage *
trafficTools_CreateTransportMessageWithSignedContentObject(RtaConnection *connection)
{
    CCNxContentObject *unsignedObject = trafficTools_CreateSignedContentObject();
    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromContentObject(unsignedObject);
    TransportMessage *tm = transportMessage_CreateFromDictionary(message);

    transportMessage_SetInfo(tm, connection, NULL);

    ccnxContentObject_Release(&unsignedObject);
    ccnxMetaMessage_Release(&message);

    return tm;
}

TransportMessage *
trafficTools_CreateTransportMessageWithSignedContentObjectWithName(RtaConnection *connection, CCNxName *name, const char *keystorePath, const char *keystorePassword)
{
    PARCBuffer *payload = parcBuffer_WrapCString("hello");

    CCNxContentObject *contentObject = ccnxContentObject_CreateWithNameAndPayload(name, payload);
    PARCBuffer *keyId = parcBuffer_WrapCString("hash of key");
    PARCBuffer *sigbits = parcBuffer_WrapCString("sig bits");
    PARCSignature *signature = parcSignature_Create(PARCSigningAlgorithm_RSA, PARCCryptoHashType_SHA256, sigbits);
    parcBuffer_Release(&sigbits);

    ccnxContentObject_SetSignature(contentObject, keyId, signature, NULL);

    CCNxMetaMessage *message = ccnxMetaMessage_CreateFromContentObject(contentObject);
    TransportMessage *tm = transportMessage_CreateFromDictionary(message);
    transportMessage_SetInfo(tm, connection, NULL);

    ccnxMetaMessage_Release(&message);
    ccnxContentObject_Release(&contentObject);
    parcSignature_Release(&signature);
    parcBuffer_Release(&keyId);
    return tm;
}

CCNxInterest *
trafficTools_CreateInterest(void)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/there/were/bells/on/the/hill");
    CCNxInterest *interest = ccnxInterest_CreateSimple(name);
    ccnxName_Release(&name);

    return interest;
}

CCNxTlvDictionary *
trafficTools_CreateDictionaryInterest(void)
{
    CCNxName *name = ccnxName_CreateFromCString("lci:/there/were/bells/on/the/hill");
    CCNxTlvDictionary *interest = ccnxInterest_CreateSimple(name);
    ccnxName_Release(&name);

    return interest;
}

TransportMessage *
trafficTools_CreateTransportMessageWithInterest(RtaConnection *connection)
{
    return trafficTools_CreateTransportMessageWithDictionaryInterest(connection, CCNxTlvDictionary_SchemaVersion_V1);
}

TransportMessage *
trafficTools_CreateTransportMessageWithControlMessage(RtaConnection *connection)
{
    return trafficTools_CreateTransportMessageWithDictionaryControl(connection, CCNxTlvDictionary_SchemaVersion_V1);
}

TransportMessage *
trafficTools_CreateTransportMessageWithRaw(RtaConnection *connection)
{
    return trafficTools_CreateTransportMessageWithDictionaryRaw(connection, CCNxTlvDictionary_SchemaVersion_V1);
}

TransportMessage *
trafficTools_CreateTransportMessageWithDictionaryInterest(RtaConnection *connection, CCNxTlvDictionary_SchemaVersion schema)
{
    CCNxTlvDictionary *interest;
    CCNxName *name = ccnxName_CreateFromCString("lci:/lost/in/space");

    CCNxInterestInterface *impl = NULL;

    switch (schema) {
        case CCNxTlvDictionary_SchemaVersion_V1:
            impl = &CCNxInterestFacadeV1_Implementation;
            break;

        default:
            trapIllegalValue(schema, "Unsupported schema version");
    }

    // impl should be set if we get here.
    interest = ccnxInterest_CreateWithImpl(impl,
                                           name,
                                           CCNxInterestDefault_LifetimeMilliseconds,
                                           NULL,
                                           NULL,
                                           CCNxInterestDefault_HopLimit);


    TransportMessage *tm = transportMessage_CreateFromDictionary(interest);
    ccnxTlvDictionary_Release(&interest);

    transportMessage_SetInfo(tm, connection, NULL);
    ccnxName_Release(&name);
    return tm;
}

TransportMessage *
trafficTools_CreateTransportMessageWithDictionaryRaw(RtaConnection *connection, unsigned schema)
{
    PARCBuffer *buffer = parcBuffer_Allocate(sizeof(v1_interest_nameA));
    parcBuffer_PutArray(buffer, sizeof(v1_interest_nameA), v1_interest_nameA);
    parcBuffer_Flip(buffer);
    CCNxTlvDictionary *wireformat = ccnxWireFormatMessage_FromInterestPacketType(schema, buffer);

    TransportMessage *tm = transportMessage_CreateFromDictionary(wireformat);
    ccnxTlvDictionary_Release(&wireformat);
    parcBuffer_Release(&buffer);

    transportMessage_SetInfo(tm, connection, NULL);
    return tm;
}

TransportMessage *
trafficTools_CreateTransportMessageWithDictionaryControl(RtaConnection *connection, unsigned schema)
{
    char *jsonstring = "{\"CPI_REQUEST\":{\"SEQUENCE\":22,\"REGISTER\":{\"PREFIX\":\"lci:/howdie/stranger\",\"INTERFACE\":55,\"FLAGS\":0,\"PROTOCOL\":\"STATIC\",\"ROUTETYPE\":\"LONGEST\",\"COST\":200}}}";

    PARCJSON *json = parcJSON_ParseString(jsonstring);
    CCNxTlvDictionary *control = NULL;

    switch (schema) {
        case 1:
            control = ccnxControlFacade_CreateCPI(json);

            break;

        default:
            break;
    }


    TransportMessage *tm = transportMessage_CreateFromDictionary(control);
    transportMessage_SetInfo(tm, connection, NULL);

    parcJSON_Release(&json);
    ccnxTlvDictionary_Release(&control);

    return tm;
}
