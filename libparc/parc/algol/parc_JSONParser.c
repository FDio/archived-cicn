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

#include <parc/assert/parc_Assert.h>

#include <ctype.h>
#include <string.h>

#include <parc/algol/parc_JSONParser.h>

#include <parc/algol/parc_BufferComposer.h>
#include <parc/algol/parc_Object.h>

struct parc_buffer_parser {
    char *ignore;
    PARCBuffer *buffer;
};

static PARCBuffer *
_getBuffer(const PARCJSONParser *parser)
{
    return parser->buffer;
}

static void
_destroyPARCBufferParser(PARCJSONParser **instancePtr)
{
    PARCJSONParser *parser = *instancePtr;
    parcBuffer_Release(&parser->buffer);
}

parcObject_ExtendPARCObject(PARCJSONParser, _destroyPARCBufferParser, NULL, NULL, NULL, NULL, NULL, NULL);

PARCJSONParser *
parcJSONParser_Create(PARCBuffer *buffer)
{
    PARCJSONParser *result = parcObject_CreateInstance(PARCJSONParser);
    result->ignore = " \t\n";
    result->buffer = parcBuffer_Acquire(buffer);
    return result;
}

void
parcJSONParser_AssertValid(const PARCJSONParser *parser)
{
    parcAssertNotNull(parser, "PARCJSONParser cannot be NULL");
    parcBuffer_OptionalAssertValid(parser->buffer);
}

parcObject_ImplementAcquire(parcJSONParser, PARCJSONParser);

parcObject_ImplementRelease(parcJSONParser, PARCJSONParser);

void
parcJSONParser_SkipIgnored(PARCJSONParser *parser)
{
    parcJSONParser_OptionalAssertValid(parser);

    parcBuffer_SkipOver(parser->buffer, strlen(parser->ignore), (uint8_t *) parser->ignore);
}

char
parcJSONParser_NextChar(PARCJSONParser *parser)
{
    parcJSONParser_SkipIgnored(parser);
    return (char) parcBuffer_GetUint8(parser->buffer);
}

bool
parcJSONParser_Next(PARCJSONParser *parser, char *value)
{
    bool result = false;
    parcJSONParser_SkipIgnored(parser);
    if (parcJSONParser_Remaining(parser) > 0) {
        *value = (char) parcBuffer_GetUint8(parser->buffer);
        result = true;
    }
    return result;
}

char
parcJSONParser_PeekNextChar(PARCJSONParser *parser)
{
    parcJSONParser_SkipIgnored(parser);
    return (char) parcBuffer_PeekByte(parser->buffer);
}

void
parcJSONParser_Advance(PARCJSONParser *parser, long bytes)
{
    parcJSONParser_OptionalAssertValid(parser);

    parcBuffer_SetPosition(parser->buffer, parcBuffer_Position(parser->buffer) + bytes);
}

size_t
parcJSONParser_Remaining(const PARCJSONParser *parser)
{
    parcJSONParser_OptionalAssertValid(parser);

    return parcBuffer_Remaining(parser->buffer);
}

bool
parcJSONParser_RequireString(PARCJSONParser *parser, const char *string)
{
    PARCBuffer *buffer = _getBuffer(parser);

    for (const char *requiredCharacter = string; *requiredCharacter != 0; requiredCharacter++) {
        uint8_t actualCharacter = parcBuffer_GetUint8(buffer);
        if (actualCharacter != *requiredCharacter) {
            return false;
        }
    }
    return true;
}

PARCBuffer *
parcJSONParser_ParseString(PARCJSONParser *parser)
{
    PARCBuffer *result = NULL;

    PARCBuffer *buffer = _getBuffer(parser);
    if (parcBuffer_GetUint8(buffer) == '"') { // skip the initial '"' character starting the string.
        PARCBufferComposer *composer = parcBufferComposer_Create();

        while (parcBuffer_Remaining(buffer)) {
            uint8_t c = parcBuffer_GetUint8(buffer);
            if (c == '"') {
                // This is the only successful way to exit this while loop.
                result = parcBufferComposer_ProduceBuffer(composer);
                break;
            } else if (c == '\\') {
                c = parcBuffer_GetUint8(buffer);
                if (c == '"') {
                    // this special character passes directly into the composed string.
                } else if (c == '\\') {
                    // this special character passes directly into the composed string.
                } else if (c == '/') {
                    // this special character passes directly into the composed string.
                } else if (c == 'b') {
                    c = '\b';
                } else if (c == 'f') {
                    c = '\f';
                } else if (c == 'n') {
                    c = '\n';
                } else if (c == 'r') {
                    c = '\r';
                } else if (c == 't') {
                    c = '\t';
                } else if (c == 'u') {
                    // Not supporting unicode at this point.
                    parcTrapNotImplemented("Unicode is not supported.");
                }
            } else if (iscntrl(c)) {
                // !! Syntax Error.
                break;
            }
            parcBufferComposer_PutChar(composer, c);
        }

        parcBufferComposer_Release(&composer);
    }
    return result;
}
