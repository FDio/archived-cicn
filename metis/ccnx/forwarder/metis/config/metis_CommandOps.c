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
#include <string.h>

#ifndef _ANDROID_
#  ifdef HAVE_ERRNO_H
#    include <errno.h>
#  else
extern int errno;
#  endif
#endif

#include <parc/algol/parc_Memory.h>

#include <ccnx/forwarder/metis/config/metis_CommandOps.h>
#include <ccnx/forwarder/metis/config/metis_CommandParser.h>

MetisCommandOps *
metisCommandOps_Create(void *closure, const char *command, void (*init)(MetisCommandParser *parser, MetisCommandOps *ops),
                       MetisCommandReturn (*execute)(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args),
                       void (*destroyer)(MetisCommandOps **opsPtr))
{
    assertNotNull(command, "Parameter command must be non-null");
    assertNotNull(execute, "Parameter execute must be non-null");
    MetisCommandOps *ops = parcMemory_AllocateAndClear(sizeof(MetisCommandOps));
    assertNotNull(ops, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisCommandOps));
    ops->closure = closure;
    ops->command = parcMemory_StringDuplicate(command, strlen(command) + 1);
    ops->init = init;
    ops->execute = execute;
    ops->destroyer = destroyer;
    return ops;
}

void
metisCommandOps_Destroy(MetisCommandOps **opsPtr)
{
    assertNotNull(opsPtr, "Parameter opsPtr must be non-null");
    assertNotNull(*opsPtr, "Parameter opsPtr must dereference to non-null pointer");

    MetisCommandOps *ops = *opsPtr;
    parcMemory_Deallocate((void **) &(ops->command));
    // DO NOT call ops->destroyer, we are one!
    parcMemory_Deallocate((void **) &ops);

    *opsPtr = NULL;
}
