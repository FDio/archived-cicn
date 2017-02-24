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
 * @file ccnx_PayloadType.h
 * @brief An enumeration for the supported types of a content object payload.
 *
 * @see CCNxContentObject
 *
 */

#ifndef libccnx_ccnx_PayloadType_h
#define libccnx_ccnx_PayloadType_h

/**
 * @typedef CCNxPayloadType
 * @brief Specifies how the Payload should be interpreted.
 */
typedef enum ccnx_payload_type {
    CCNxPayloadType_DATA = 0,
    CCNxPayloadType_KEY = 1,
    CCNxPayloadType_LINK = 2,
    CCNxPayloadType_MANIFEST = 3
} CCNxPayloadType;
#endif /* defined(libccnx_ccnx_PayloadType_h) */
