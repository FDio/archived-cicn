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
 * @file ccnxCodecSchemaV1_ManifestEncoder.h
 * @brief Encode a V1 Manifest.
 *
 */

#ifndef ccnx_common_ccnxCodecSchemaV1_ManifestEncoder_h
#define ccnx_common_ccnxCodecSchemaV1_ManifestEncoder_h

#include <ccnx/common/internal/ccnx_TlvDictionary.h>
#include <ccnx/common/codec/ccnxCodec_TlvEncoder.h>

ssize_t ccnxCodecSchemaV1ManifestEncoder_Encode(CCNxCodecTlvEncoder *encoder, CCNxTlvDictionary *packetDictionary);

#endif // ccnx_common_ccnxCodecSchemaV1_ManifestEncoder_h
