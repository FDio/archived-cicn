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
 * @file cpi_private.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */

#ifndef libccnx_cpi_private_h
#define libccnx_cpi_private_h

uint64_t cpi_GetNextSequenceNumber(void);

/**
 * Wrap the operation in a CPI_REQUEST and add sequence number
 *
 *   <#Discussion#>
 *
 * @param <#param1#>
 * @return JSON "{ CPI_REQUEST: { SEQUENCE:number key: { operation } }}"
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCJSON *cpi_CreateRequest(const char *key, PARCJSON *operation);

/**
 * <#OneLineDescription#>
 *
 *   INPUT: "{ CPI_REQUEST: { SEQUENCE:number key: { operation } }}"
 *   OUTPUT: "{ key : { operation } }"
 *
 *   Example: "{ REGISTER : { <code>cpiRoute_ToJson()</code> } }"
 * @param <#param1#>
 * @return <#return#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCJSONPair *cpi_ParseRequest(PARCJSON *request);
#endif // libccnx_cpi_private_h
