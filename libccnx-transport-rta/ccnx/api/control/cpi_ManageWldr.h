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

#ifndef libccnx_cpi_ManageWldr_h
#define libccnx_cpi_ManageWldr_h

#include <parc/algol/parc_JSON.h>

struct cpi_manage_wldr;
/**
 * @typedef CPIManageWldr
 */
typedef struct cpi_manage_wldr CPIManageWldr;

/**
 */

void cpiManageWldr_Destroy(CPIManageWldr **cpiWldrPtr);

CPIManageWldr *cpiManageWldr_Create(bool active, char *conn);

char *CPIManageWldr_ToString(CPIManageWldr *cpiWldr);

CPIManageWldr *cpiManageWldr_Copy(const CPIManageWldr *original);

bool cpiManageWldr_Equals(const CPIManageWldr *a, const CPIManageWldr *b);

bool cpiManageWldr_IsActive(const CPIManageWldr *cpiWldr);

const char *cpiManageWldr_GetConnection(const CPIManageWldr *cpiWldr);

PARCJSON *cpiManageWldr_ToJson(const CPIManageWldr *cpiWldr);


CPIManageWldr *cpiManageWldr_FromJson(PARCJSON *json);

#endif // libccnx_cpi_ManageWldr_h
