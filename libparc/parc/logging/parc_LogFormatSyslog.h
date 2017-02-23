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
 * @file parc_LogFormatSyslog.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef __PARC_Library__parc_LogFormatSyslog__
#define __PARC_Library__parc_LogFormatSyslog__

#include <parc/algol/parc_Buffer.h>
#include <parc/logging/parc_LogEntry.h>

PARCBuffer *parcLogFormatSyslog_FormatEntry(const PARCLogEntry *entry);

#endif /* defined(__PARC_Library__parc_LogFormatSyslog__) */
