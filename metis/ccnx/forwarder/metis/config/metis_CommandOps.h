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
 * @file metis_CommandOps.h
 * @brief The function structure defining a CLI command
 *
 * The function structure that defines a CLI command.  Each command will return one
 * of these which defines how to run the command.
 *
 */

#ifndef Metis_metis_CommandOps_h
#define Metis_metis_CommandOps_h

#include <parc/algol/parc_List.h>

#include <ccnx/forwarder/metis/config/metis_CommandReturn.h>

// forward reference
struct metis_command_parser;

struct metis_command_ops;
typedef struct metis_command_ops MetisCommandOps;

/**
 * @typedef MetisCommandOps
 * @abstract Each command implements a MetisCommandOps
 * @constant closure is a user-specified pointer for any state the user needs
 * @constant command The text string of the command, must be the spelled out string, e.g. "help list routes"
 * @constant init A function to call to initialize the command at program startup
 * @constant execute A function to call to execute the command
 * @constant destroyer A function to call to release the command
 * @discussion
 *     Typically, the root of the thee has an Init function that then initilizes the
 *     rest of the tree.  For example:
 *
 * @code
 *    const MetisCommandOps metisControl_Root = {
 *      .closure = NULL,
 *      .command = "", // empty string for root
 *      .init    = metisControl_Root_Init,
 *      .execute = metisControl_Root_Execute
 *      .destroyer = NULL
 *    };
 * @endcode
 *
 * The metisControl_Root_Init function will then begin adding the subtree under root.  For example:
 *
 * @code
 *  const MetisCommandOps metisControl_Add = {
 *      .closure = NULL,
 *      .command = "add",
 *      .init    = metisControl_Add_Init,
 *      .execute = metisControl_Add_Execute,
 *      .destroyer = NULL
 *  };
 *
 *  static void
 *  metisControl_Root_Init(MetisControlState *state, MetisCommandOps *ops)
 *  {
 *      metisControlState_RegisterCommand(state, &metisControl_Add);
 *  }
 * @endcode
 */
struct metis_command_ops {
    void *closure;
    char *command;
    void (*init)(struct metis_command_parser *parser, MetisCommandOps *ops);
    MetisCommandReturn (*execute)(struct metis_command_parser *parser, MetisCommandOps *ops, PARCList *args);
    void (*destroyer)(MetisCommandOps **opsPtr);
};

/**
 * A helper function to create the pubically defined MetisCommandOps.
 *
 * Retruns allocated memory of the command
 *
 * @param [in] command The string is copied
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisCommandOps *metisCommandOps_Create(void *closure,
                                        const char *command,
                                        void (*init)(struct metis_command_parser *parser, MetisCommandOps *ops),
                                        MetisCommandReturn (*execute)(struct metis_command_parser *parser, MetisCommandOps *ops, PARCList *args),
                                        void (*destroyer)(MetisCommandOps **opsPtr));

/**
 * De-allocates the memory of the MetisCommandOps and the copied command string
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @retval <#value#> <#explanation#>
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisCommandOps_Destroy(MetisCommandOps **opsPtr);
#endif // Metis_metis_CommandOps_h
