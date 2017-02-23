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

#include <parc/security/parc_Security.h>

#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_List.h>
#include <parc/algol/parc_TreeRedBlack.h>
#include <parc/algol/parc_Time.h>

#include <ccnx/common/ccnx_KeystoreUtilities.h>

#include <ccnx/forwarder/metis/config/metis_CommandParser.h>

#ifndef _ANDROID_
#  ifdef HAVE_ERRNO_H
#    include <errno.h>
#  else
extern int errno;
#  endif
#endif

struct metis_command_parser {
    // key = command, value = MetisCommandOps
    PARCTreeRedBlack *commandTree;
    bool debugFlag;
};

static int
_stringCompare(const void *key1, const void *key2)
{
    return strcasecmp((const char *) key1, (const char *) key2);
}

MetisCommandParser *
metisCommandParser_Create(void)
{
    MetisCommandParser *state = parcMemory_AllocateAndClear(sizeof(MetisCommandParser));
    assertNotNull(state, "parcMemory_AllocateAndClear(%zu) returned NULL", sizeof(MetisCommandParser));
    state->commandTree = parcTreeRedBlack_Create(
        _stringCompare,                                           // key compare
        NULL,                                                     // key free
        NULL,                                                     // key copy
        NULL,                                                     // value equals
        NULL,                                                     // value free
        NULL                                                      // value copy
        );
    state->debugFlag = false;
    return state;
}

void
metisCommandParser_Destroy(MetisCommandParser **parserPtr)
{
    MetisCommandParser *parser = *parserPtr;

    // destroy every element if it has a destroyer
    PARCArrayList *values = parcTreeRedBlack_Values(parser->commandTree);
    if (values) {
        for (int i = 0; i < parcArrayList_Size(values); i++) {
            MetisCommandOps *ops = parcArrayList_Get(values, i);
            parcTreeRedBlack_Remove(parser->commandTree, ops->command);
            if (ops->destroyer) {
                ops->destroyer(&ops);
            }
        }
        parcArrayList_Destroy(&values);
    }

    parcTreeRedBlack_Destroy(&parser->commandTree);

    parcMemory_Deallocate((void **) &parser);
    *parserPtr = NULL;
}

void
metisCommandParser_SetDebug(MetisCommandParser *state, bool debugFlag)
{
    state->debugFlag = debugFlag;
}

bool
metisCommandParser_GetDebug(MetisCommandParser *state)
{
    return state->debugFlag;
}

void
metisCommandParser_RegisterCommand(MetisCommandParser *state, MetisCommandOps *ops)
{
    assertNotNull(state, "Parameter state must be non-null");
    assertNotNull(ops, "Parameter ops must be non-null");
    assertNotNull(ops->command, "Operation command string must be non-null");

    void *exists = parcTreeRedBlack_Get(state->commandTree, ops->command);
    assertNull(exists, "Command '%s' already exists in the tree %p\n", ops->command, (void *) exists);

    parcTreeRedBlack_Insert(state->commandTree, (void *) ops->command, (void *) ops);

    // if the command being registered asked for an init function to be called, call it
    if (ops->init != NULL) {
        ops->init(state, ops);
    }
}

static PARCList *
parseStringIntoTokens(const char *originalString)
{
    PARCList *list = parcList(parcArrayList_Create(parcArrayList_StdlibFreeFunction), PARCArrayListAsPARCList);

    char *token;

    char *tofree = parcMemory_StringDuplicate(originalString, strlen(originalString) + 1);
    char *string = tofree;

    while ((token = strsep(&string, " \t\n")) != NULL) {
        if (strlen(token) > 0) {
            parcList_Add(list, strdup(token));
        }
    }

    parcMemory_Deallocate((void **) &tofree);

    return list;
}

/**
 * Matches the user arguments to available commands, returning the command or NULL if not found
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static MetisCommandOps *
metisCommandParser_MatchCommand(MetisCommandParser *state, PARCList *args)
{
    // Find the longest matching prefix command.
    // Pretty wildly inefficient

    size_t longest_token_count = 0;
    char *longest_command = NULL;

    PARCArrayList *commands = parcTreeRedBlack_Keys(state->commandTree);
    for (int i = 0; i < parcArrayList_Size(commands); i++) {
        char *command = parcArrayList_Get(commands, i);
        PARCList *command_tokens = parseStringIntoTokens(command);

        // is it a prefix match?
        if (parcList_Size(args) >= parcList_Size(command_tokens)) {
            bool possible_match = true;
            for (int i = 0; i < parcList_Size(command_tokens) && possible_match; i++) {
                const char *a = parcList_GetAtIndex(command_tokens, i);
                const char *b = parcList_GetAtIndex(args, i);
                if (strncasecmp(a, b, strlen(a) + 1) != 0) {
                    possible_match = false;
                }
            }

            if (possible_match && parcList_Size(command_tokens) > longest_token_count) {
                longest_token_count = parcList_Size(command_tokens);
                longest_command = command;
            }
        }

        parcList_Release(&command_tokens);
    }
    
    parcArrayList_Destroy(&commands);

    if (longest_token_count == 0) {
        return NULL;
    } else {
        MetisCommandOps *ops = parcTreeRedBlack_Get(state->commandTree, longest_command);
        assertNotNull(ops, "Got null operations for command '%s'\n", longest_command);
        return ops;
    }
}

MetisCommandReturn
metisCommandParser_DispatchCommand(MetisCommandParser *state, PARCList *args)
{
    MetisCommandOps *ops = metisCommandParser_MatchCommand(state, args);

    if (ops == NULL) {
        printf("Command not found.\n");
        return MetisCommandReturn_Failure;
    } else {
        return ops->execute(state, ops, args);
    }
}

bool
metisCommandParser_ContainsCommand(MetisCommandParser *parser, const char *command)
{
    MetisCommandOps *ops = parcTreeRedBlack_Get(parser->commandTree, command);
    return (ops != NULL);
}
