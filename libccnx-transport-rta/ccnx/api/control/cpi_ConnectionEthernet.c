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

#include <ccnx/api/control/cpi_ConnectionEthernet.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_JSON.h>

#include <ccnx/api/control/controlPlaneInterface.h>
extern uint64_t cpi_GetNextSequenceNumber(void);

// JSON keys
static const char *KEY_IFNAME = "IFNAME";
static const char *KEY_ADDR = "PEER_ADDR";
static const char *KEY_ETHERTYPE = "ETHERTYPE";
static const char *KEY_SYMBOLIC = "SYMBOLIC";

static const char *KEY_ADDETHER = "AddConnEther";
static const char *KEY_REMOVEETHER = "RemoveConnEther";

struct cpi_connection_ethernet {
    char *interfaceName;
    char *symbolic;
    CPIAddress *peerLinkAddress;
    uint16_t ethertype;
};

CPIConnectionEthernet *
cpiConnectionEthernet_Create(const char *interfaceName, CPIAddress *peerLinkAddress, uint16_t ethertype, const char *symbolic)
{
    assertNotNull(interfaceName, "Parameter interfaceName must be non-null");
    assertNotNull(peerLinkAddress, "Parameter peerLinkAddress must be non-null");

    CPIConnectionEthernet *etherConn = parcMemory_AllocateAndClear(sizeof(CPIConnectionEthernet));
    if (etherConn) {
        etherConn->interfaceName = parcMemory_StringDuplicate(interfaceName, strlen(interfaceName));
        etherConn->symbolic = parcMemory_StringDuplicate(symbolic, strlen(symbolic));
        etherConn->peerLinkAddress = cpiAddress_Copy(peerLinkAddress);
        etherConn->ethertype = ethertype;
    }

    return etherConn;
}

void
cpiConnectionEthernet_Release(CPIConnectionEthernet **etherConnPtr)
{
    assertNotNull(etherConnPtr, "Parameter etherConnPtr must be non-null double pointer");
    assertNotNull(*etherConnPtr, "Parameter etherConnPtr dereference to non-null pointer");

    CPIConnectionEthernet *etherConn = *etherConnPtr;
    cpiAddress_Destroy(&etherConn->peerLinkAddress);
    parcMemory_Deallocate((void **) &(etherConn->interfaceName));
    parcMemory_Deallocate((void **) &(etherConn->symbolic));
    parcMemory_Deallocate((void **) &etherConn);
    *etherConnPtr = NULL;
}

bool
cpiConnectionEthernet_Equals(const CPIConnectionEthernet *a, const CPIConnectionEthernet *b)
{
    if ((a == NULL && b == NULL) || a == b) {
        // both null or identically equal
        return true;
    }

    if (a == NULL || b == NULL) {
        // only one is null
        return false;
    }

    if (a->ethertype == b->ethertype) {
        if (cpiAddress_Equals(a->peerLinkAddress, b->peerLinkAddress)) {
            if (strcmp(a->interfaceName, b->interfaceName) == 0) {
                if (strcmp(a->symbolic, b->symbolic) == 0) {
                    return true;
                }
            }
        }
    }
    return false;
}


static PARCJSON *
_cpiConnectionEthernet_ToJson(const CPIConnectionEthernet *etherConn)
{
    PARCJSON *json = parcJSON_Create();

    // ------ Interface Name
    parcJSON_AddString(json, KEY_IFNAME, etherConn->interfaceName);

    // ------ Symbolic Name
    parcJSON_AddString(json, KEY_SYMBOLIC, etherConn->symbolic);

    // ------ Link Address
    PARCJSON *peerLinkJson = cpiAddress_ToJson(etherConn->peerLinkAddress);
    parcJSON_AddObject(json, KEY_ADDR, peerLinkJson);
    parcJSON_Release(&peerLinkJson);

    // ------ EtherType
    parcJSON_AddInteger(json, KEY_ETHERTYPE, etherConn->ethertype);

    return json;
}

/*
 * We want to create a JSON object that looks like this
 *  {
 *     "CPI_REQUEST" :
 *        {  "SEQUENCE" : <sequence number>,
 *           <operationName> : { "IFNAME" : "em1", "SYMBOLIC" : "conn0", "PEER_ADDR" : { "ADDRESSTYPE" : "LINK", "DATA" : "AQIDBAUG" }, "ETHERTYPE" : 2049 },
 *        }
 *  }
 */
static CCNxControl *
_cpiConnectionEthernet_CreateControlMessage(const CPIConnectionEthernet *etherConn, const char *operationName)
{
    PARCJSON *cpiRequest = parcJSON_Create();

    // --- add the seqnum

    uint64_t seqnum = cpi_GetNextSequenceNumber();
    parcJSON_AddInteger(cpiRequest, "SEQUENCE", (int) seqnum);

    // -- Add the operation
    PARCJSON *operation = _cpiConnectionEthernet_ToJson(etherConn);
    parcJSON_AddObject(cpiRequest, operationName, operation);
    parcJSON_Release(&operation);

    // -- Do the final encapusulation
    PARCJSON *final = parcJSON_Create();
    parcJSON_AddObject(final, cpiRequest_GetJsonTag(), cpiRequest);
    parcJSON_Release(&cpiRequest);

    // -- Create the CPIControlMessage
    char *finalString = parcJSON_ToCompactString(final);

    parcJSON_Release(&final);

    PARCJSON *oldJson = parcJSON_ParseString(finalString);
    CCNxControl *result = ccnxControl_CreateCPIRequest(oldJson);
    parcJSON_Release(&oldJson);

    parcMemory_Deallocate((void **) &finalString);

    return result;
}

CCNxControl *
cpiConnectionEthernet_CreateAddMessage(const CPIConnectionEthernet *etherConn)
{
    assertNotNull(etherConn, "Parameter etherConn must be non-null");
    CCNxControl *control = _cpiConnectionEthernet_CreateControlMessage(etherConn, KEY_ADDETHER);
    return control;
}

CCNxControl *
cpiConnectionEthernet_CreateRemoveMessage(const CPIConnectionEthernet *etherConn)
{
    assertNotNull(etherConn, "Parameter etherConn must be non-null");
    CCNxControl *control = _cpiConnectionEthernet_CreateControlMessage(etherConn, KEY_REMOVEETHER);
    return control;
}

static bool
_cpiConnectionEthernet_IsMessageType(const CCNxControl *control, const char *operationName)
{
    bool isOperation = false;
    if (ccnxControl_IsCPI(control)) {
        PARCJSON *oldJson = ccnxControl_GetJson(control);
        PARCJSONValue *value = parcJSON_GetValueByName(oldJson, cpiRequest_GetJsonTag());

        if (value != NULL) {
            PARCJSON *innerJson = parcJSONValue_GetJSON(value);
            // the second array element is the key we're looking for
            PARCJSONPair *pair = parcJSON_GetPairByIndex(innerJson, 1);
            if (pair != NULL) {
                const char *opKey = parcBuffer_Overlay(parcJSONPair_GetName(pair), 0);
                if (opKey && strcasecmp(opKey, operationName) == 0) {
                    isOperation = true;
                }
            }
        }
    }

    return isOperation;
}

bool
cpiConnectionEthernet_IsAddMessage(const CCNxControl *control)
{
    assertNotNull(control, "Parameter control must be non-null");
    return _cpiConnectionEthernet_IsMessageType(control, KEY_ADDETHER);
}

bool
cpiConnectionEthernet_IsRemoveMessage(const CCNxControl *control)
{
    assertNotNull(control, "Parameter control must be non-null");
    return _cpiConnectionEthernet_IsMessageType(control, KEY_REMOVEETHER);
}

CPIConnectionEthernet *
cpiConnectionEthernet_FromControl(const CCNxControl *control)
{
    assertNotNull(control, "Parameter control must be non-null");

    CPIConnectionEthernet *etherConn = NULL;

    if (ccnxControl_IsCPI(control)) {
        PARCJSON *oldJson = ccnxControl_GetJson(control);
        PARCJSONValue *value = parcJSON_GetValueByName(oldJson, cpiRequest_GetJsonTag());

        if (value != NULL) {
            assertTrue(parcJSONValue_IsJSON(value),
                       "Wrong JSON type for %s, expected JSON: %s",
                       cpiRequest_GetJsonTag(), parcJSON_ToString(oldJson));
            PARCJSON *requestJson = parcJSONValue_GetJSON(value);
            // the second array element is the key we're looking for
            PARCJSONPair *pair = parcJSON_GetPairByIndex(requestJson, 1);
            const char *opKey = parcBuffer_Overlay(parcJSONPair_GetName(pair), 0);
            if (opKey && ((strcasecmp(opKey, KEY_ADDETHER) == 0) || strcasecmp(opKey, KEY_REMOVEETHER))) {
                PARCJSON *opJson = parcJSONValue_GetJSON(parcJSONPair_GetValue(pair));

                // Ok, it is one of our messages, now assemble the pieces
                value = parcJSON_GetValueByName(opJson, KEY_IFNAME);
                PARCBuffer *sBuf = parcJSONValue_GetString(value);
                const char *ifname = parcBuffer_Overlay(sBuf, 0);
                value = parcJSON_GetValueByName(opJson, KEY_SYMBOLIC);
                sBuf = parcJSONValue_GetString(value);
                const char *symbolic = parcBuffer_Overlay(sBuf, 0);
                value = parcJSON_GetValueByName(opJson, KEY_ETHERTYPE);
                int ethertype = (int) parcJSONValue_GetInteger(value);
                value = parcJSON_GetValueByName(opJson, KEY_ADDR);
                PARCJSON *addrJson = parcJSONValue_GetJSON(value);
                assertNotNull(addrJson, "JSON missing the key %s", KEY_ADDR);

                CPIAddress *peerAddress = cpiAddress_CreateFromJson(addrJson);
                assertNotNull(peerAddress, "Failed to decode the peer address from %s", parcJSON_ToString(addrJson));

                etherConn = cpiConnectionEthernet_Create(ifname, peerAddress, (uint16_t) ethertype, symbolic);

                cpiAddress_Destroy(&peerAddress);
            }
        }
    }

    return etherConn;
}

const char *
cpiConnectionEthernet_GetInterfaceName(const CPIConnectionEthernet *etherConn)
{
    assertNotNull(etherConn, "Parameter etherConn must be non-null");
    return etherConn->interfaceName;
}

const char *
cpiConnectionEthernet_GetSymbolicName(const CPIConnectionEthernet *etherConn)
{
    assertNotNull(etherConn, "Parameter etherConn must be non-null");
    return etherConn->symbolic;
}

CPIAddress *
cpiConnectionEthernet_GetPeerLinkAddress(const CPIConnectionEthernet *etherConn)
{
    assertNotNull(etherConn, "Parameter etherConn must be non-null");
    return etherConn->peerLinkAddress;
}

uint16_t
cpiConnectionEthernet_GetEthertype(const CPIConnectionEthernet *etherConn)
{
    assertNotNull(etherConn, "Parameter etherConn must be non-null");
    return etherConn->ethertype;
}
