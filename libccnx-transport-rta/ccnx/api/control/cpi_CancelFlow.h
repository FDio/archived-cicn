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
 * @file cpi_CancelFlow.h
 * @brief Cancel a "flow"
 *
 */
#ifndef libccnx_cpi_CancelFlow_h
#define libccnx_cpi_CancelFlow_h

#include <ccnx/api/control/cpi_ControlMessage.h>

#include <ccnx/common/ccnx_Name.h>

/**
 * Creates a CPI reqeust to cancel a flow
 *
 * Will return an asynchronous ACK or NACK.
 *
 * @param name The CCNxName of the flow to cancel.
 * @return A pointer to a valid CPIControlMessage
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCJSON *cpiCancelFlow_CreateRequest(const CCNxName *name);

/**
 * Creates a CPI reqeust to cancel a flow
 *
 * @param [in] name The CCNxName of the flow to cancel.
 *
 * @return NULL An error occurred
 * @return non-NULL A pointer to a valid PARCJSON instance.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCJSON *cpiCancelFlow_Create(const CCNxName *name);

/**
 * Return the CCNxName associated with the given control message
 *
 * @param controlMessage A pointer to a control message.
 * @return A pointer to a valid CCNxName instance.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxName *cpiCancelFlow_GetFlowName(const PARCJSON *controlMessage);

/**
 * Return the name associated with the message
 *
 * @param controlMessage A pointer to a control message.
 * @return A pointer to a valid CCNxName instance.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
CCNxName *cpiCancelFlow_NameFromControlMessage(CCNxControl *controlMessage);

/**
 * Given a CPI response (ACK or NACK) return the success state
 *
 * @param controlMessage A pointer to a control message.
 * @return true if the control message signals success.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool cpiCancelFlow_SuccessFromResponse(CCNxControl *controlMessage);

/**
 * The CPI tag used for cancel flow
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *cpiCancelFlow_CancelFlowJsonTag(void);
#endif // libccnx_cpi_CancelFlow_h
