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
 * @file ccnx_ContentObjectFacade.h
 * @ingroup ContentObject
 * @brief This facade is used to access fields within an RTA-encoded content object.
 *
 * Content objects are encoded and transmitted through the transport stack before being sent over the wire.
 * This facade acts an interface to this transport-specific encoding of the content object. It enables
 * the user to directly access fields within the content object without having any knowledge about the
 * particular schema-specific encoding.
 *
 */
#ifndef libccnx_ccnx_ContentObjectFacadeV1_h
#define libccnx_ccnx_ContentObjectFacadeV1_h

#include <ccnx/common/internal/ccnx_ContentObjectInterface.h>

extern CCNxContentObjectInterface CCNxContentObjectFacadeV1_Implementation;

#endif // libccnx_ccnx_ContentObjectFacadeV1_h
