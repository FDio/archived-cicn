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
 * @file ANSITerm/longBowReport_Runtime.h
 * @ingroup reporting
 * @brief ANSI Terminal Reporting
 *
 */
#ifndef LongBow_longBowReport_ANSITerm_Runtime_h
#define LongBow_longBowReport_ANSITerm_Runtime_h

#include <stdio.h>
#include <stdarg.h>

#include <LongBow/Reporting/longBowReport_Runtime.h>

/**
 * Begin printing in red.
 *
 */
void longBowReportRuntime_PrintRed(void);

/**
 * Print the formatted string in red.
 *
 */
void longBowReportRuntime_RedPrintf(const char *format, ...);

/**
 * Print the formatted string in green.
 *
 */
void longBowReportRuntime_GreenPrintf(const char *format, ...);

/**
 * Print the formatted string in yellow.
 *
 */
void longBowReportRuntime_YellowPrintf(const char *format, ...);

/**
 * Print the formatted string in magenta.
 *
 */
void longBowReportRuntime_MagentaPrintf(const char *format, ...);

/**
 * Parse the given key and set the corresponding LongBowReportConfig to suppress reports.
 *
 * @param [in] config A valid LongBowReportConfig instance.
 * @param [in] key A nul-terminated C string consisting of one or more of the characters, X.SWstwFTU
 */
void longBowReportRuntime_ParseSuppress(LongBowReportConfig *config, const char *key);
#endif // LongBow_longBowReport_ANSITerm_Runtime_h
