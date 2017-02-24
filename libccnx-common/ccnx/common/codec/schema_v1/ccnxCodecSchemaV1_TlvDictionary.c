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
 */

#include <config.h>
#include <stdio.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>

#include <LongBow/runtime.h>
#include <ccnx/common/ccnx_PayloadType.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_TlvDictionary.h>

CCNxTlvDictionary *
ccnxCodecSchemaV1TlvDictionary_CreateInterest(void)
{
    CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(CCNxCodecSchemaV1TlvDictionary_MessageFastArray_END, CCNxCodecSchemaV1TlvDictionary_Lists_END);
    ccnxTlvDictionary_SetMessageType_Interest(dictionary, CCNxTlvDictionary_SchemaVersion_V1);
    return dictionary;
}

CCNxTlvDictionary *
ccnxCodecSchemaV1TlvDictionary_CreateContentObject(void)
{
    CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(CCNxCodecSchemaV1TlvDictionary_MessageFastArray_END, CCNxCodecSchemaV1TlvDictionary_Lists_END);
    ccnxTlvDictionary_SetMessageType_ContentObject(dictionary, CCNxTlvDictionary_SchemaVersion_V1);
    return dictionary;
}

CCNxTlvDictionary *
ccnxCodecSchemaV1TlvDictionary_CreateManifest(void)
{
    CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(CCNxCodecSchemaV1TlvDictionary_MessageFastArray_END, CCNxCodecSchemaV1TlvDictionary_Lists_END);
    ccnxTlvDictionary_SetMessageType_Manifest(dictionary, CCNxTlvDictionary_SchemaVersion_V1);

    return dictionary;
}

CCNxTlvDictionary *
ccnxCodecSchemaV1TlvDictionary_CreateControl(void)
{
    CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(CCNxCodecSchemaV1TlvDictionary_MessageFastArray_END, CCNxCodecSchemaV1TlvDictionary_Lists_END);
    ccnxTlvDictionary_SetMessageType_Control(dictionary, CCNxTlvDictionary_SchemaVersion_V1);
    return dictionary;
}

CCNxTlvDictionary *
ccnxCodecSchemaV1TlvDictionary_CreateInterestReturn(void)
{
    CCNxTlvDictionary *dictionary = ccnxTlvDictionary_Create(CCNxCodecSchemaV1TlvDictionary_MessageFastArray_END, CCNxCodecSchemaV1TlvDictionary_Lists_END);
    ccnxTlvDictionary_SetMessageType_InterestReturn(dictionary, CCNxTlvDictionary_SchemaVersion_V1);
    return dictionary;
}

