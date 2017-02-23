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
 * @file parc_DisplayIndented.h
 * @ingroup developer
 * @brief Support for displaying information on the console.
 *
 *
 */
#ifndef libparc_parc_DisplayIndented_h
#define libparc_parc_DisplayIndented_h

#include <stdarg.h>
#include <stdlib.h>

/**
 * Print an indented, formatted string on standard output.
 *
 * The line is automatically terminated with a new line.
 *
 * @param [in] indentation The indentation level of the output.
 * @param [in] format The format string.
 * @param [in] ... A variable number of arguments.
 *
 * Example:
 * @code
 * {
 *     parcDisplayIndented_PrintLine(2, "This is printed on standard output, at indentation level 2");
 * }
 * @endcode
 */
void parcDisplayIndented_PrintLine(int indentation, const char *format, ...);

/**
 * Print memory.
 *
 * @param [in] indentation The indentation level of the output.
 * @param [in] length The length of the array.
 * @param [in] memory The memory array.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
void parcDisplayIndented_PrintMemory(int indentation, size_t length, const char *memory);
#endif // libparc_parc_DisplayIndented_h
