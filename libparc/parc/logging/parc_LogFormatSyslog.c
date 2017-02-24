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

#include <inttypes.h>

#include <parc/logging/parc_LogFormatSyslog.h>
#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_Time.h>

/*
 * RFC 5424
 *
 * SYSLOG-MSG      = HEADER SP STRUCTURED-DATA [SP MSG]
 *
 * HEADER          = PRI VERSION SP TIMESTAMP SP HOSTNAME SP APP-NAME SP PROCID SP MSGID
 * PRI             = "<" PRIVAL ">"
 * PRIVAL          = 1*3DIGIT ; range 0 .. 191
 * VERSION         = NONZERO-DIGIT 0*2DIGIT
 * HOSTNAME        = NILVALUE / 1*255PRINTUSASCII
 *
 * APP-NAME        = NILVALUE / 1*48PRINTUSASCII
 * PROCID          = NILVALUE / 1*128PRINTUSASCII
 * MSGID           = NILVALUE / 1*32PRINTUSASCII
 *
 * TIMESTAMP       = NILVALUE / FULL-DATE "T" FULL-TIME
 * FULL-DATE       = DATE-FULLYEAR "-" DATE-MONTH "-" DATE-MDAY
 * DATE-FULLYEAR   = 4DIGIT
 * DATE-MONTH      = 2DIGIT  ; 01-12
 * DATE-MDAY       = 2DIGIT  ; 01-28, 01-29, 01-30, 01-31 based on month/year
 * FULL-TIME       = PARTIAL-TIME TIME-OFFSET
 * PARTIAL-TIME    = TIME-HOUR ":" TIME-MINUTE ":" TIME-SECOND
 * [TIME-SECFRAC]
 * TIME-HOUR       = 2DIGIT  ; 00-23
 * TIME-MINUTE     = 2DIGIT  ; 00-59
 * TIME-SECOND     = 2DIGIT  ; 00-59
 * TIME-SECFRAC    = "." 1*6DIGIT
 * TIME-OFFSET     = "Z" / TIME-NUMOFFSET
 * TIME-NUMOFFSET  = ("+" / "-") TIME-HOUR ":" TIME-MINUTE
 *
 *
 * STRUCTURED-DATA = NILVALUE / 1*SD-ELEMENT
 * SD-ELEMENT      = "[" SD-ID *(SP SD-PARAM) "]"
 * SD-PARAM        = PARAM-NAME "=" %d34 PARAM-VALUE %d34
 * SD-ID           = SD-NAME
 * PARAM-NAME      = SD-NAME
 * PARAM-VALUE     = UTF-8-STRING ; characters '"', '\' and ']' MUST be escaped.
 * SD-NAME         = 1*32PRINTUSASCII ; except '=', SP, ']', %d34 (")
 *
 * MSG             = MSG-ANY / MSG-UTF8
 * MSG-ANY         = *OCTET ; not starting with BOM
 * MSG-UTF8        = BOM UTF-8-STRING
 * BOM             = %xEF.BB.BF
 *
 *
 * UTF-8-STRING    = *OCTET ; UTF-8 string as specified in RFC 3629
 *
 * OCTET           = %d00-255
 * SP              = %d32
 * PRINTUSASCII    = %d33-126
 * NONZERO-DIGIT   = %d49-57
 * DIGIT           = %d48 / NONZERO-DIGIT
 * NILVALUE        = "-"
 */
/**
 * Create a PARCBuffer containing the PARCLogEntry formatted according to RFC 5424 section 6.
 *
 * The returned PARCBuffer's position is set to the start of the formatted data and continues to the limit.
 *
 * @param [in] entry A pointer to a valid instance of PARCLogEntry.
 *
 * @return non-NULL A pointer to a PARCBuffer containing the formatted PARCLogEntry.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 *
 * @see parcLogEntry_ToString
 */
PARCBuffer *
parcLogFormatSyslog_FormatEntry(const PARCLogEntry *entry)
{
    PARCBuffer *payload = parcLogEntry_GetPayload(entry);

    char theTime[64];
    parcTime_TimevalAsRFC3339(parcLogEntry_GetTimeStamp(entry), theTime);

    PARCBufferComposer *composer = parcBufferComposer_Allocate(128);

    parcBufferComposer_Format(composer, "<%s> %d ",
                              parcLogLevel_ToString(parcLogEntry_GetLevel(entry)), parcLogEntry_GetVersion(entry));
    parcBufferComposer_PutStrings(composer,
                                  theTime, " ",
                                  parcLogEntry_GetHostName(entry), " ",
                                  parcLogEntry_GetApplicationName(entry), " ",
                                  parcLogEntry_GetProcessName(entry), " ", NULL);

    parcBufferComposer_Format(composer, "%" PRId64 " [ ", parcLogEntry_GetMessageId(entry));
    parcBufferComposer_PutBuffer(composer, payload);
    parcBufferComposer_PutStrings(composer, " ]\n", NULL);
    PARCBuffer *result = parcBuffer_Flip(parcBuffer_Acquire(parcBufferComposer_GetBuffer(composer)));

    parcBufferComposer_Release(&composer);

    return result;
}
