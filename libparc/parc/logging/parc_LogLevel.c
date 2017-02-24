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
 */
#include <config.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include <parc/logging/parc_LogLevel.h>

const PARCLogLevel PARCLogLevel_Off = 0;

const PARCLogLevel PARCLogLevel_All = 255;

const PARCLogLevel PARCLogLevel_Emergency = 1;

const PARCLogLevel PARCLogLevel_Alert = 2;

const PARCLogLevel PARCLogLevel_Critical = 3;

const PARCLogLevel PARCLogLevel_Error = 4;

const PARCLogLevel PARCLogLevel_Warning = 5;

const PARCLogLevel PARCLogLevel_Notice = 6;

const PARCLogLevel PARCLogLevel_Info = 7;

const PARCLogLevel PARCLogLevel_Debug = 8;

static char *levelToString[] = {
    "Off",
    "Emergency",
    "Alert",
    "Critical",
    "Error",
    "Warning",
    "Notice",
    "Info",
    "Debug",
    NULL
};

//int
//parcLogLevel_Compare(const PARCLogLevel levelA, const PARCLogLevel levelB)
//{
//    return levelA - levelB;
//}

int
parcLogLevel_Equals(const PARCLogLevel levelA, const PARCLogLevel levelB)
{
    return levelA == levelB;
}


PARCLogLevel
parcLogLevel_FromString(const char *levelAsString)
{
    PARCLogLevel result = PARCLogLevel_All;
    for (size_t i = 0; levelToString[i] != NULL; i++) {
        if (strcasecmp(levelAsString, levelToString[i]) == 0) {
            result = i;
        }
    }

    return result;
}

const char *
parcLogLevel_ToString(const PARCLogLevel level)
{
    char *result = "All";
    if (level <= PARCLogLevel_Debug) {
        result = levelToString[level];
    }
    return result;
}
