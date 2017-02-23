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

#include <config.h>
#include <stdio.h>

#include <ccnx/api/control/controlPlaneInterface.h>
#include <ccnx/api/control/cpi_ManageCaches.h>
#include <LongBow/runtime.h>

#include "cpi_private.h"

static const char *cpiCacheStoreOn = "CACHE_STORE_ON";
static const char *cpiCacheStoreOff = "CACHE_STORE_OFF";
static const char *cpiCacheServeOn = "CACHE_SERVE_ON";
static const char *cpiCacheServeOff = "CACHE_SERVE_OFF";
static const char *cpiCacheClear = "CACHE_CLEAR";

PARCJSON *
cpiManageChaces_CreateCacheClearRequest()
{
    PARCJSON *json = parcJSON_Create();
    PARCJSON *result = cpi_CreateRequest(cpiCacheClear, json);
    parcJSON_Release(&json);

    return result;
}


PARCJSON *
cpiManageChaces_CreateCacheStoreRequest(bool activate)
{
    PARCJSON *json = parcJSON_Create();
    PARCJSON *result;
    if (activate) {
        result = cpi_CreateRequest(cpiCacheStoreOn, json);
    } else {
        result = cpi_CreateRequest(cpiCacheStoreOff, json);
    }
    parcJSON_Release(&json);

    return result;
}


PARCJSON *
cpiManageChaces_CreateCacheServeRequest(bool activate)
{
    PARCJSON *json = parcJSON_Create();
    PARCJSON *result;
    if (activate) {
        result = cpi_CreateRequest(cpiCacheServeOn, json);
    } else {
        result = cpi_CreateRequest(cpiCacheServeOff, json);
    }
    parcJSON_Release(&json);

    return result;
}


const char *
cpiManageChaces_CacheStoreOnJsonTag()
{
    return cpiCacheStoreOn;
}

const char *
cpiManageChaces_CacheStoreOffJsonTag()
{
    return cpiCacheStoreOff;
}

const char *
cpiManageChaces_CacheServeOnJsonTag()
{
    return cpiCacheServeOn;
}

const char *
cpiManageChaces_CacheServeOffJsonTag()
{
    return cpiCacheServeOff;
}

const char *
cpiManageChaces_CacheClearJsonTag()
{
    return cpiCacheClear;
}

