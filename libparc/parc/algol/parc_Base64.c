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
 *
 * Encode/decode base64.  Encoding goes to one long line, no line breaks.
 * Decoding will accept CRLF linebreaks in the data and skip them.
 *
 * Following the language of RFC 4648, encoding proceeds in a "quantum" of
 * 3 bytes of plaintext to 4 bytes of encoded data.  Decoding goes in
 * a 4-byte quantum to 3-byte decoded data.
 *
 * If decoding fails (e.g. there's a non-base64 character), then the output
 * buffer is rewound to the starting position and a failure is indicated.
 *
 * Decoding using a 256 byte table.  Each byte of the 4-byte quantum is looked
 * up and if its a valid character -- it resolves to a value 0..63, then that
 * value is shifted to the right position in the output.  Values CR and LF have
 * the special token "_" in the table, which means "skip".  That token has value
 * ascii value 95, is we can detect it as outside base64.  Similarly, all the
 * invalid characters have the symbol "~", which is ascii 127.
 *
 */

#include <config.h>

#include <stdio.h>
#include <string.h>

#include <parc/assert/parc_Assert.h>

#include <parc/algol/parc_Base64.h>
#include <parc/algol/parc_Memory.h>

const uint8_t base64code[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
const uint8_t pad = '=';

const uint8_t invalid = '~';       // has ascii value 127, outside base64
const uint8_t skip = '_';           // has ascii value 95, outside the base64 values

// an 256-entry table to lookup decode values.  If the value is "invalid", then it's not
// a base64 character.
const uint8_t decodeTable[256] = {
/*   0 */ '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '_', '~', '~', '_', '~', '~',
/*  16 */ '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
/*  32 */ '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', 62,  '~', '~', '~', 63,
/*  48 */ 52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  '~', '~', '~', '~', '~', '~',
/*  64 */ '~', 0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,
/*  80 */ 15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  '~', '~', '~', '~', '~',
/*  96 */ '~', 26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
/* 112 */ 41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  '~', '~', '~', '~', '~',
/* 128 */ '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
    '~',       '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
    '~',       '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
    '~',       '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
    '~',       '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
    '~',       '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
    '~',       '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~',
    '~',       '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~', '~'
};

#define min(a, b) ((a < b) ? a : b)

/**
 * Encode the 3-byte quantum pointed to by <code>quantum</code> into 4 encoded characters.
 * It includes `padLength` of pad necessary at the end.
 */
static void
_encodeWithPad(PARCBufferComposer *output, const uint8_t *quantum, size_t padLength)
{
    parcAssertTrue(padLength < 3, "Degenerate case -- should never pad all 3 bytes!");

    unsigned index;

    uint8_t paddedQuantum[] = { 0, 0, 0 };
    memcpy(paddedQuantum, quantum, 3 - padLength);

    for (index = 0; index < 4; index++) {
        if (index + padLength < 4) {
            /*
             * The four base64 symbols fall in to these locations in the
             * 3-byte input
             *
             * aaaaaabb | bbbbcccc | ccdddddd
             *
             * This switch statement, based on the "a" "b" "c" or "d" case
             * extracts the corresponding 6 bits from its location in the
             * byte aray.
             */
            int sixbit = 0;
            switch (index) {
                case 0: // aaaaaa
                    sixbit = paddedQuantum[0] >> 2;
                    break;

                case 1: // bbbbbb
                    sixbit = ((paddedQuantum[0] & 0x03) << 4) | (paddedQuantum[1] >> 4);
                    break;

                case 2: // cccccc
                    sixbit = ((paddedQuantum[1] & 0x0F) << 2) | (paddedQuantum[2] >> 6);
                    break;

                case 3: // dddddd
                    sixbit = paddedQuantum[2] & 0x3F;
                    break;
            }
            uint8_t encodedChar = base64code[ sixbit ];
            parcBufferComposer_PutUint8(output, encodedChar);
        } else {
            parcBufferComposer_PutUint8(output, pad);
        }
    }
}

/**
 * Decode the 4-byte quantum of base64 to binary.
 */
static bool
_decode(PARCBufferComposer *output, uint8_t *quantum)
{
    uint8_t threebytes[3] = { 0, 0, 0 };
    size_t length_to_append = 0;

    for (int index = 0; index < 4; index++) {
        uint8_t c = quantum[index];
        if (c != pad) {
            uint8_t value = decodeTable[c];

            // if its a non-base64 character, bail out of here
            if (value == invalid) {
                return false;
            }

            /*
             * The four base64 symbols fall in to these locations in the
             * final 3-byte output
             *
             * aaaaaabb | bbbbcccc | ccdddddd
             */
            switch (index) {
                case 0: // aaaaaa
                    threebytes[0] |= value << 2;
                    break;

                case 1: // bbbbbb
                    threebytes[0] |= (value & 0x30) >> 4;
                    threebytes[1] |= (value & 0x0F) << 4;

                    // we've finished threebytes[0]
                    length_to_append = 1;
                    break;

                case 2: // cccccc
                    threebytes[1] |= value >> 2;
                    threebytes[2] |= (value & 0x03) << 6;

                    // we've finished threebytes[1]
                    length_to_append = 2;
                    break;

                case 3: // dddddd
                    threebytes[2] |= value;

                    // we've finished threebytes[2]
                    length_to_append = 3;
                    break;
            }
        }
    }

    parcBufferComposer_PutArray(output, threebytes, length_to_append);
    return true;
}

PARCBufferComposer *
parcBase64_Encode(PARCBufferComposer *result, PARCBuffer *plainText)
{
    size_t remaining = parcBuffer_Remaining(plainText);
    if (remaining > 0) {
        const uint8_t *buffer = parcBuffer_Overlay(plainText, 0);
        result = parcBase64_EncodeArray(result, remaining, buffer);
    }

    return result;
}

PARCBufferComposer *
parcBase64_EncodeArray(PARCBufferComposer *output, size_t length, const uint8_t array[length])
{
    size_t offset = 0;

    // Encode 3-byte tuples
    while (offset < length) {
        const uint8_t *quantum = array + offset;
        size_t padLength = 3 - min(3, length - offset);
        _encodeWithPad(output, quantum, padLength);
        offset += 3;
    }

    return output;
}

PARCBufferComposer *
parcBase64_Decode(PARCBufferComposer *output, PARCBuffer *encodedText)
{
    // We proceed in 4-byte blocks.  All base-64 encoded data is a multiple of 4 bytes.
    // If the length of encodedText is wrong, bail now

    size_t remaining = parcBuffer_Remaining(encodedText);
    const uint8_t *buffer = parcBuffer_Overlay(encodedText, remaining);
    return parcBase64_DecodeArray(output, remaining, buffer);
}

PARCBufferComposer *
parcBase64_DecodeString(PARCBufferComposer *output, const char *encodedString)
{
    const uint8_t *buffer = (const uint8_t *) encodedString;
    size_t length = strlen(encodedString);
    return parcBase64_DecodeArray(output, length, buffer);
}

PARCBufferComposer *
parcBase64_DecodeArray(PARCBufferComposer *output, size_t length, const uint8_t array[length])
{
    size_t offset = 0;
    bool success = true;

    // if we need to rollback, this is where we go
    PARCBuffer *outputBuffer = parcBufferComposer_GetBuffer(output);
    size_t rewind_to = parcBuffer_Position(outputBuffer);

    while (offset < length && success) {
        // filter out line feeds and carrage returns
        // parse the input in 4-byte quantums
        size_t index = 0;
        uint8_t quantum[4];

        // reset success at the start of each loop.  if we run out of input before
        // we parse a full quantum, we'll fail the loop and rewind the output buffer.
        success = false;

        // 4 == quantum length for decode
        while (index < 4 && offset < length) {
            uint8_t c = array[offset];
            uint8_t decoded = decodeTable[c];

            if (decoded < 64 || c == pad) {
                // this is an artifact from how the code was first written, so we
                // pass the un-decoded character
                quantum[index] = c;
                index++;
                offset++;
                continue;
            }

            if (decoded == skip) {
                offset++;
                continue;
            }

            if (decoded == invalid) {
                break;
            }
        }

        if (index == 4) {
            success = _decode(output, quantum);
        }
    }

    if (!success) {
        parcBuffer_SetPosition(outputBuffer, rewind_to);
        return NULL;
    }

    return output;
}
