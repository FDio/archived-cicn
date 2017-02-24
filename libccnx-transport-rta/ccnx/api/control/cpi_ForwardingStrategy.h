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

#ifndef libccnx_cpi_ForwardingStrategy_h
#define libccnx_cpi_ForwardingStrategy_h

#include <ccnx/common/ccnx_Name.h>
#include <ccnx/api/control/cpi_Address.h>

#include <parc/algol/parc_JSON.h>

struct cpi_forwarding_strategy;
/**
 * @typedef CPIForwardingStrategy
 * @brief A representation of a forwarding strategy.
 */
typedef struct cpi_forwarding_strategy CPIForwardingStrategy;

/**
 */


void cpiForwardingStrategy_Destroy(CPIForwardingStrategy **fwdStrategyPtr);

CPIForwardingStrategy *cpiForwardingStrategy_Create(CCNxName *prefix, char *strategy);

char *cpiForwardingStrategy_ToString(CPIForwardingStrategy *fwdStrategy);

CPIForwardingStrategy *cpiForwardingStrategy_Copy(const CPIForwardingStrategy *original);

bool cpiForwardingStrategy_Equals(const CPIForwardingStrategy *a, const CPIForwardingStrategy *b);

const CCNxName *cpiForwardingStrategy_GetPrefix(const CPIForwardingStrategy *fwdStrategy);

const char *cpiForwardingStrategy_GetStrategy(const CPIForwardingStrategy *fwdStrategy);

PARCJSON *cpiForwardingStrategy_ToJson(const CPIForwardingStrategy *fwdStrategy);

CPIForwardingStrategy *cpiForwardingStrategy_FromJson(PARCJSON *json);

#endif // libccnx_cpi_ForwardingStrategy_h
