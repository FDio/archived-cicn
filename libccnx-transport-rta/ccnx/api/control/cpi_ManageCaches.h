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

#ifndef libccnx_cpi_ManageCaches_h
#define libccnx_cpi_ManageCaches_h


PARCJSON *cpiManageChaces_CreateCacheStoreRequest(bool activate);
PARCJSON *cpiManageChaces_CreateCacheServeRequest(bool activate);
PARCJSON *cpiManageChaces_CreateCacheClearRequest();

//const char *cpiLinks_AddEtherConnectionJasonTag();

const char *cpiManageChaces_CacheStoreOnJsonTag();

const char *cpiManageChaces_CacheStoreOffJsonTag();

const char *cpiManageChaces_CacheServeOnJsonTag();

const char *cpiManageChaces_CacheServeOffJsonTag();

const char *cpiManageChaces_CacheClearJsonTag();
#endif
