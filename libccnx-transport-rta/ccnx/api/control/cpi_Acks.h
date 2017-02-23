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
 * @file cpi_Acks.h
 *
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
#ifndef libccnx_cpi_Acks_h
#define libccnx_cpi_Acks_h

#include <stdbool.h>
#include <stdint.h>

#include <parc/algol/parc_JSON.h>

struct control_plane_ack;
typedef struct control_plane_ack CPIAck;

#define cpiAck      "CPI_ACK"
#define cpiSeqnum   "SEQUENCE"

/**
 * Create a CPIAck instance from a PARCJSON instance.
 *
 * @param [in] json A pointer to a valid PARCJSON instance.
 *
 * @return NULL An error occurred.
 * @return non-NULL A pointer to a valid CPIAck instance.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 */
CPIAck *cpiAcks_ParseJSON(const PARCJSON *json);

/**
 * Create a CPIAck instance.
 *
 * @param [in] sequenceNumber The sequence number for the ACK.
 *
 * @return NULL An error occurred.
 * @return non-NULL A pointer to a valid CPIAck instance.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 *
 */
CPIAck *cpiAcks_Create(uint64_t sequenceNumber);

/**
 * Create a CPIAck instance, from a template of the original request.
 *
 * @param [in] originalRequest A pointer to a valid PARCJSON instance containint he original request.
 *
 * @return NULL An error occurred.
 * @return non-NULL A pointer to a valid PARCJSON instance.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 *
 */
PARCJSON *cpiAcks_CreateAck(const PARCJSON *originalRequest);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 *
 */
PARCJSON *cpiAcks_CreateNack(const PARCJSON *request);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool cpiAcks_IsAck(const PARCJSON *json);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 *
 */
uint64_t cpiAcks_GetAckOriginalSequenceNumber(const PARCJSON *json);
#endif // libccnx_cpi_Acks_h
