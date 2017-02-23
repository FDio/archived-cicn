
#include <config.h>
#include <stdio.h>
#include "config_Codec_Ccnb.h"

#include "components.h"

/**
 * Generates:
 
 { "CCNB_CODEC" : { } }
 */
ProtocolStackConfig *
ccnbCodec_ProtocolStackConfig(ProtocolStackConfig *stackConfig)
{
    return protocolStackConfig_Add(stackConfig, ccnbCodec_GetName(), parcJSONValue_CreateNULL());//ccnxJson_CreateObject());
}

ConnectionConfig *
ccnbCodec_ConnectionConfig(ConnectionConfig *connectionConfig)
{
    return connectionConfig_Add(connectionConfig, ccnbCodec_GetName(), parcJSONValue_CreateNULL());//ccnxJson_CreateObject());
}


const char * ccnbCodec_GetName()
{
    return RtaComponentNames[CODEC_CCNB];

}

