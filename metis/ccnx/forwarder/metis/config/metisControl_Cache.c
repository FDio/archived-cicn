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

#include <stdbool.h>
#include <stdint.h>
#include <strings.h>
#include <stdlib.h>
#include <stdio.h>

#include <LongBow/runtime.h>

#include <parc/security/parc_Security.h>

#include <parc/algol/parc_Memory.h>

#include <ccnx/forwarder/metis/config/metisControl_Cache.h>
#include <ccnx/forwarder/metis/config/metisControl_CacheServe.h>
#include <ccnx/forwarder/metis/config/metisControl_CacheStore.h>
#include <ccnx/forwarder/metis/config/metisControl_CacheClear.h>

static void _metisControlCache_Init(MetisCommandParser *parser, MetisCommandOps *ops);
static MetisCommandReturn _metisControlCache_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);
static MetisCommandReturn _metisControlCache_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args);

static const char *_commandCache = "cache";
static const char *_commandCacheHelp = "help cache";

MetisCommandOps *
metisControlCache_Create(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandCache, _metisControlCache_Init, _metisControlCache_Execute, metisCommandOps_Destroy);
}

MetisCommandOps *
metisControlCache_HelpCreate(MetisControlState *state)
{
    return metisCommandOps_Create(state, _commandCacheHelp, NULL, _metisControlCache_HelpExecute, metisCommandOps_Destroy);
}

// =====================================================

static MetisCommandReturn
_metisControlCache_HelpExecute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    MetisCommandOps *ops_cache_serve = metisControlCacheServe_HelpCreate(NULL);
    MetisCommandOps *ops_cache_store = metisControlCacheStore_HelpCreate(NULL);
    MetisCommandOps *ops_cache_clear = metisControlCacheClear_HelpCreate(NULL);

    printf("Available commands:\n");
    printf("   %s\n", ops_cache_serve->command);
    printf("   %s\n", ops_cache_store->command);
    printf("   %s\n", ops_cache_clear->command);
    printf("\n");

    metisCommandOps_Destroy(&ops_cache_serve);
    metisCommandOps_Destroy(&ops_cache_store);
    metisCommandOps_Destroy(&ops_cache_clear);

    return MetisCommandReturn_Success;
}

static void
_metisControlCache_Init(MetisCommandParser *parser, MetisCommandOps *ops)
{
    MetisControlState *state = ops->closure;
    metisControlState_RegisterCommand(state, metisControlCacheServe_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlCacheStore_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlCacheClear_HelpCreate(state));
    metisControlState_RegisterCommand(state, metisControlCacheServe_Create(state));
    metisControlState_RegisterCommand(state, metisControlCacheStore_Create(state));
    metisControlState_RegisterCommand(state, metisControlCacheClear_Create(state));
}

static MetisCommandReturn
_metisControlCache_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
{
    return _metisControlCache_HelpExecute(parser, ops, args);
}

// ======================================================================
