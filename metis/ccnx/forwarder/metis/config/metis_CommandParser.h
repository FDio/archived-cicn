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
 * @file metis_CommandParser.h
 * @brief Creates a dictionary of commands and parses a command-line to match against them
 *
 * A user creates individual CommandParserEntry that map a command-line to a function
 * to execute.  The CommandParser then does a longest-matching prefix match of a command-line
 * to the dictionary of commands and executes the appropriate command.
 *
 */

#ifndef Metis_metis_command_parser_h
#define Metis_metis_command_parser_h

#include <ccnx/transport/common/transport_MetaMessage.h>

#include <ccnx/forwarder/metis/config/metis_CommandReturn.h>
#include <ccnx/forwarder/metis/config/metis_CommandOps.h>

struct metis_command_parser;
typedef struct metis_command_parser MetisCommandParser;

/**
 * metisControlState_Create
 *
 * Creates the global state for the MetisControl program
 *
 * @return non-null A command parser
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisCommandParser *metisCommandParser_Create(void);

/**
 * Destroys the control state, closing all network connections
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
void metisCommandParser_Destroy(MetisCommandParser **statePtr);

/**
 * Registers a MetisCommandOps with the system.
 *
 * Each command has its complete command prefix in the "command" field.  RegisterCommand
 * will put these command prefixes in to a tree and then match what a user types against
 * the longest-matching prefix in the tree.  If there's a match, it will call the "execute"
 * function.
 *
 * When the parser is destroyed, each command's destroyer function will be called.
 *
 * @param [in] state An allocated MetisControlState
 * @param [in] command The command to register with the system
 *
 * Example:
 * @code
 *      static MetisControlReturn
 *      metisControl_Root_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
 *      {
 *          printf("Root Command\n");
 *          return MetisCommandReturn_Success;
 *      }
 *
 *      static MetisControlReturn
 *      metisControl_FooBar_Execute(MetisCommandParser *parser, MetisCommandOps *ops, PARCList *args)
 *      {
 *          printf("Foo Bar Command\n");
 *          return MetisCommandReturn_Success;
 *      }
 *
 *      const MetisCommandOps metisControl_Root = {
 *      .closure = NULL,
 *      .command = "", // empty string for root
 *      .init    = NULL,
 *      .execute = metisControl_Root_Execute
 *      };
 *
 *      const MetisCommandOps metisControl_FooBar = {
 *      .closure = NULL,
 *      .command = "foo bar", // empty string for root
 *      .init    = NULL,
 *      .execute = metisControl_FooBar_Execute
 *      };
 *
 *   void startup(void)
 *   {
 *      MetisControlState *state = metisControlState_Create("happy", "day");
 *      metisControlState_RegisterCommand(state, metisControl_FooBar);
 *      metisControlState_RegisterCommand(state, metisControl_Root);
 *
 *      // this executes "root"
 *      metisControlState_DispatchCommand(state, "foo");
 *      metisControlState_Destroy(&state);
 *  }
 * @endcode
 */
void metisCommandParser_RegisterCommand(MetisCommandParser *state, MetisCommandOps *command);

/**
 * Performs a longest-matching prefix of the args to the command tree
 *
 * The command tree is created with metisControlState_RegisterCommand.
 *
 * @param [in] state The allocated MetisControlState
 * @param [in] args  Each command-line word parsed to the ordered list
 *
 * @return MetisCommandReturn_Success the command was successful
 * @return MetisCommandReturn_Failure the command failed or was not found
 * @return MetisCommandReturn_Exit the command indicates that the interactive mode should exit
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisCommandReturn metisCommandParser_DispatchCommand(MetisCommandParser *state, PARCList *args);

/**
 * Sets the Debug mode, which will print out much more information.
 *
 * Prints out much more diagnostic information about what metis-control is doing.
 * yes, you would make a MetisCommandOps to set and unset this :)
 *
 * @param [in] debugFlag true means to print debug info, false means to turn it off
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisCommandParser_SetDebug(MetisCommandParser *state, bool debugFlag);

/**
 * Returns the debug state of MetisControlState
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
bool metisCommandParser_GetDebug(MetisCommandParser *state);

/**
 * Checks if the command is registered
 *
 * Checks if the exact command given is registered.  This is not a prefix match.
 *
 * @param [<#in out in,out#>] <#name#> <#description#>
 *
 * @return true The command is registered
 * @return false The command is not registered
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool metisCommandParser_ContainsCommand(MetisCommandParser *parser, const char *command);
#endif // Metis_metis_command_parser_h
