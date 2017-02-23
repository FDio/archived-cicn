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
 * @file ccnx_ChunkingFacade.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */

#ifndef libccnx_ccnx_ChunkingFacadeV1_h
#define libccnx_ccnx_ChunkingFacadeV1_h

#include <ccnx/common/internal/ccnx_TlvDictionary.h>

/**
 * Determines if an EndChunkNumber exists in the metadata
 *
 * Compatible with V0 and V1 schemas.
 *
 * @param [in] contentObjectDictionary A dictionary to check
 *
 * @return false There is no EndChunkNumber specified or the dictinoary is not a ContentObject
 * @return true There is an EndChunkNumber specified
 *
 * Example:
 * @code
 *      static PARCElasticBuffer *
 *      _ccnxContentObjectFacade_CreateFinalBlockId(const CCNxTlvDictionary *contentObjectDictionary)
 *      {
 *          if (ccnxChunkingFacade_HasEndChunkNumber(contentObjectDictionary)) {
 *              uint64_t endChunkNumber = ccnxChunkingFacade_GetEndChunkNumber(contentObjectDictionary);
 *              PARCElasticBuffer *fbid = _ccnxContentObjectFacade_EncodeFinalBlockId(endChunkNumber);
 *              return fbid;
 *          }
 *          return NULL;
 *      }
 * @endcode
 */
bool ccnxChunkingFacadeV1_HasEndChunkNumber(const CCNxTlvDictionary *contentObjectDictionary);

/**
 * Retrieves the end chunk number
 *
 * Retieves the end chunk number as an unsigned 64-bit integer.
 * This function will trapIllegalValue if there is not an EndChunkNumber present.
 * The EndChunkNumber is the chunk number of the last Content Object in a chunked series.
 *
 * @param [in] contentObjectDictionary A dictionary to check
 *
 * @return number The ending chunk number
 *
 * Example:
 * @code
 *      static PARCElasticBuffer *
 *      _ccnxContentObjectFacade_CreateFinalBlockId(const CCNxTlvDictionary *contentObjectDictionary)
 *      {
 *          if (ccnxChunkingFacade_HasEndChunkNumber(contentObjectDictionary)) {
 *              uint64_t endChunkNumber = ccnxChunkingFacade_GetEndChunkNumber(contentObjectDictionary);
 *              PARCElasticBuffer *fbid = _ccnxContentObjectFacade_EncodeFinalBlockId(endChunkNumber);
 *              return fbid;
 *          }
 *          return NULL;
 *      }
 * @endcode
 */
uint64_t ccnxChunkingFacadeV1_GetEndChunkNumber(const CCNxTlvDictionary *contentObjectDictionary);

/**
 * Sets the EndChunkNumber of a ContentObject
 *
 * The dictionary must be a ContentObject, otherwise this function will trapIllegalValue.
 * If an EndChunkNumber already exits, will not update but return false.
 *
 * @param [in] contentObjectDictionary A dictionary to check
 * @param [in] endChunkNumber The ending chunk number
 *
 * @return true The value was set in the dictionary
 * @return false A failure (likely value already set)
 *
 * Example:
 * @code
 * {
 *          CCNxTlvDictionary *obj = ccnxContentObjectFacade_Create(...);
 *          ccnxChunkingFacade_SetEndChunkNumber(obj, 74);
 * }
 * @endcode
 */
bool ccnxChunkingFacadeV1_SetEndChunkNumber(CCNxTlvDictionary *contentObjectDictionary, uint64_t endChunkNumber);
#endif // libccnx_ccnx_ChunkingFacadeV1_h
