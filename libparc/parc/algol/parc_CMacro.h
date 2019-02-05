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
 * @file parc_CMacro.h
 * @ingroup Macro
 * @brief Utility C-Macros
 *
 * CMacro contains a set of utility macros for doing complicated macro processing
 *
 */
#ifndef libparc_parc_CMacro_h
#define libparc_parc_CMacro_h

/**
 * parcCMacro_ThridParam expands to the "3rd" input parameter whatever that may be. It is a part
 * of the c-macro trick for implement a macro If-Else switch. If the first argument expands
 * to a comma then this macro expands to the second parameter, otherwise it expands to the 3rd parameter.
 */
#define parcCMacro_ThirdParam(A, B, C, ...) C

/**
 * parcCMacro_IfElse is a c-macro trick for implementing a macro If-Else switch.
 * It uses parcCMacro_ThirdParam to select between A or B depending on whether __VA_ARGS__ expands to a comma.
 */


#define EXPAND( x ) x
#define parcCMacro_IfElse(A, B, ...) EXPAND(parcCMacro_ThirdParam( __VA_ARGS__ , A, B, NOOP))

/** \cond */
#define _parcCMacro_Cat_(A, B) A ## B
/** \endcond */

/**
 * parcCMacro_Cat combines two strings into a single token.
 */
#define parcCMacro_Cat(A, B) _parcCMacro_Cat_(A, B)

#endif //libparc_parc_CMacro_h
