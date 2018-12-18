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

#include <LongBow/runtime.h>

#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <inttypes.h>

#include <parc/algol/parc_JSONValue.h>
#include <parc/algol/parc_JSON.h>

#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Object.h>

typedef enum {
    PARCJSONValueType_Boolean,
    PARCJSONValueType_String,
    PARCJSONValueType_Number,
    PARCJSONValueType_Array,
    PARCJSONValueType_JSON,
    PARCJSONValueType_Null
} _PARCJSONValueType;

struct parc_json_value {
    _PARCJSONValueType type;

    union {
        bool boolean;
        PARCBuffer *string;
        int64_t intValue;
        PARCList *_array;
        PARCJSONArray *array;
        PARCJSON *object;
        struct {
            bool internalDoubleRepresentation;
            long double internalDoubleValue;

            int sign;
            int64_t whole;
            int64_t fraction;
            int64_t fractionLog10;
            int64_t exponent;
        } number;
    } value;
};



static void
_parcJSONValueDestroy(PARCJSONValue **valuePtr)
{
    if (valuePtr != NULL) {
        PARCJSONValue *value = *valuePtr;
        if (value->type == PARCJSONValueType_Array) {
            parcJSONArray_Release(&value->value.array);
        } else if (value->type == PARCJSONValueType_JSON) {
            parcJSON_Release(&value->value.object);
        } else if (value->type == PARCJSONValueType_String) {
            parcBuffer_Release(&value->value.string);
        }
    }
}

parcObject_ExtendPARCObject(PARCJSONValue, _parcJSONValueDestroy, NULL, NULL, parcJSONValue_Equals, NULL, NULL, NULL);

static PARCJSONValue *
_createValue(_PARCJSONValueType type)
{
    PARCJSONValue *result = parcObject_CreateAndClearInstance(PARCJSONValue);

    if (result != NULL) {
        result->type = type;
    }
    return result;
}

/**
 * Return true if the parser is currently positioned at the valid beginning of a number.
 * If true, then return the sign (-1, +1) in the integer pointed to by @p sign.
 * If false, then return false ensuring that the parser is repositioned to where it started.
 */
static bool
_parseSign(PARCJSONParser *parser, int *sign)
{
    if (parcJSONParser_Remaining(parser) > 0) {
        uint8_t c = parcJSONParser_NextChar(parser);
        if (c == '-') {
            *sign = -1;
            return true;
        }
        if (!isdigit(c)) {
            return false;
        }
        parcJSONParser_Advance(parser, -1);
    }
    *sign = 1;
    return true;
}

static PARCJSONValue *
_parcJSONValue_FalseParser(PARCJSONParser *parser)
{
    PARCJSONValue *result = NULL;

    if (parcJSONParser_RequireString(parser, "false")) {
        result = parcJSONValue_CreateFromBoolean(false);
    }
    return result;
}

static PARCJSONValue *
_parcJSONValue_NullParser(PARCJSONParser *parser)
{
    PARCJSONValue *result = NULL;

    if (parcJSONParser_RequireString(parser, "null")) {
        result = parcJSONValue_CreateFromNULL();
    }
    return result;
}

static PARCJSONValue *
_parcJSONValue_TrueParser(PARCJSONParser *parser)
{
    PARCJSONValue *result = NULL;

    if (parcJSONParser_RequireString(parser, "true")) {
        result = parcJSONValue_CreateFromBoolean(true);
    }
    return result;
}

static PARCJSONValue *
_parcJSONValue_StringParser(PARCJSONParser *parser)
{
    PARCJSONValue *result = NULL;
    PARCBuffer *string = parcJSONParser_ParseString(parser);

    if (string != NULL) {
        result = parcJSONValue_CreateFromString(string);
        parcBuffer_Release(&string);
    }

    return result;
}

static int
_digittoint(char digit)
{
    return digit - '0';
}

/*
 * Parse the whole number portion of a number.
 *
 * 0
 * [1-9][0-9]*
 */
static bool
_parseWholeNumber(PARCJSONParser *parser, int64_t *value)
{
    bool result = false;
    int sign = 1;

    char nextCharacter;

    if (parcJSONParser_Next(parser, &nextCharacter)) {
        if (nextCharacter == '0') {
            *value = 0;
            result = true;
        } else if (isdigit(nextCharacter)) {
            *value = _digittoint(nextCharacter);
            while (parcJSONParser_Next(parser, &nextCharacter)) {
                if (!isdigit(nextCharacter)) {
                    parcJSONParser_Advance(parser, -1);
                    break;
                }
                *value = *value * 10 + _digittoint(nextCharacter);
            }
            *value = *value * sign;
            result = true;
        }
    }

    return result;
}

static bool
_parseFractionNumber(PARCJSONParser *parser, int64_t *value, int *log10)
{
    bool result = false;

    if (parcJSONParser_Remaining(parser) > 0) {
        *value = 0;
        *log10 = 0;
        char nextCharacter;
        while (parcJSONParser_Next(parser, &nextCharacter)) {
            if (!isdigit(nextCharacter)) {
                parcJSONParser_Advance(parser, -1);
                break;
            }
            *value = *value * 10 + _digittoint(nextCharacter);
            *log10 = *log10 + 1;
        }

        result = true;
    }

    return result;
}

/**
 * Parse an optional fractional part of a number.
 *
 * If the parser is positioned at a '.' character, then parse a fraction comprised of numbers.
 * Otherwise, if the parser is positioned at a 'e' ',' ']' or '}' then there is no fraction, but not an error.
 * If the parser is positioned at any other character, then it is a syntax error.
 *
 * @param [in] parser A pointer to a PARCJSONParser instance.
 * @param [out] value A pointer to an integer accumulating the fraction as a whole number.
 * @param [out] log10 A pointer to an integer accumulating the base 10 logarithm of the fraction (as a positive integer).
 *
 * @return true If there was no syntax error.
 * @return false If there was a syntax error.
 */
static bool
_parseOptionalFraction(PARCJSONParser *parser, int64_t *value, int *log10)
{
    bool result = true;

    // The parser is either looking at an '.' which signals the start of a fractional part,
    // or a 'e'  ',' ']' or '}' which signals a missing fractional part.
    // Any other character would be the beginning of a syntax error.

    char nextCharacter;

    if (parcJSONParser_Next(parser, &nextCharacter)) {
        if (nextCharacter == '.') {
            if (_parseFractionNumber(parser, value, log10) == false) {
                result = false;
            }
        } else if (nextCharacter == 'e' || nextCharacter == ',' || nextCharacter == ']' || nextCharacter == '}') {
            parcJSONParser_Advance(parser, -1);
            result = true;
        } else {
            parcJSONParser_Advance(parser, -1);
            result = false;
        }
    }

    return result;
}

/**
 * Parse and compute the base 10 value of a a sequence of digits from 0 to 9, inclusive.
 *
 * @param [in] parser A pointer to a PARCJSONParser instance.
 * @param [out] value A pointer to a value that will receive the base 10 value.
 *
 * @return true If there were parsable digits.
 */
static bool
_parseDigits09(PARCJSONParser *parser, int64_t *value)
{
    bool result = false;

    *value = 0;
    char nextDigit;
    while (parcJSONParser_Next(parser, &nextDigit)) {
        *value = *value * 10 + _digittoint(nextDigit);
        result = true;
    }

    return result;
}

static bool
_parseExponentNumber(PARCJSONParser *parser, int64_t *value)
{
    bool result = false;
    int sign = 1;

    char nextCharacter;
    if (parcJSONParser_Next(parser, &nextCharacter)) {
        if (nextCharacter == '-') {
            sign = -1;
            if (_parseDigits09(parser, value)) {
                result = true;
            }
            *value = *value * sign;
        } else if (nextCharacter == '+') {
            sign = 1;
            if (_parseDigits09(parser, value)) {
                result = true;
            }
            *value = *value * sign;
        } else if (isdigit(nextCharacter)) {
            parcJSONParser_Advance(parser, -1);
            if (_parseDigits09(parser, value)) {
                result = true;
            }
            *value = *value * sign;
        } else {
            result = false;
        }
    }

    return result;
}

static bool
_parseOptionalExponent(PARCJSONParser *parser, int64_t *value)
{
    // The parser is either looking at a 'e' or 'E'  ','  ']' or '}'
    bool result = true;

    char nextCharacter;
    if (parcJSONParser_Next(parser, &nextCharacter)) {
        if (nextCharacter == 'e' || nextCharacter == 'E') {
            if (_parseExponentNumber(parser, value) == false) {
                result = false;
            }
        } else if (nextCharacter == ',' || nextCharacter == ']' || nextCharacter == '}') {
            parcJSONParser_Advance(parser, -1);
            result = true;
        } else {
            parcJSONParser_Advance(parser, -1);
            result = false;
        }
    }

    return result;
}

static __attribute__ ((noinline))  PARCJSONValue *
_parcJSONValue_CreateNumber(int sign, int64_t whole, int64_t fraction, int64_t fractionLog10, int64_t exponent)
{
    PARCJSONValue *result = _createValue(PARCJSONValueType_Number);
    if (result != NULL) {
        result->value.number.sign = sign;
        result->value.number.whole = whole;
        result->value.number.fraction = fraction;
        result->value.number.fractionLog10 = fractionLog10;
        result->value.number.exponent = exponent;
    }
    return result;
}

static PARCJSONValue *
_parcJSONValue_NumberParser(PARCJSONParser *parser)
{
    PARCJSONValue *result = NULL;
    int sign = 1;
    int64_t whole = 0;
    int64_t fraction = 0;
    int64_t exponent = 0;
    int fractionLog10 = 0;

    if (_parseSign(parser, &sign)) {
        if (_parseWholeNumber(parser, &whole)) {
            if (_parseOptionalFraction(parser, &fraction, &fractionLog10)) {
                if (_parseOptionalExponent(parser, &exponent)) {
                    result = _parcJSONValue_CreateNumber(sign, whole, fraction, fractionLog10, exponent);
                }
            }
        }
    }

    return result;
}

static PARCJSONValue *
_parcJSONValue_ArrayParser(PARCJSONParser *parser)
{
    PARCJSONValue *result = NULL;

    if (parcJSONParser_NextChar(parser) == '[') {
        PARCJSONArray *array = parcJSONArray_Create();

        while (parcJSONParser_Remaining(parser)) {
            char peek = parcJSONParser_PeekNextChar(parser);
            if (peek == ',') {
                parcJSONParser_NextChar(parser);
            } else if (peek == ']') {
                parcJSONParser_NextChar(parser); // absorb the ']' character
                result = parcJSONValue_CreateFromJSONArray(array);
                parcJSONArray_Release(&array);
                break;
            } else {
                PARCJSONValue *value = NULL;

                if (peek == 'n') {
                    value = _parcJSONValue_NullParser(parser);
                } else if (peek == 't') {
                    value = _parcJSONValue_TrueParser(parser);
                } else if (peek == 'f') {
                    value = _parcJSONValue_FalseParser(parser);
                } else if (peek == '"') {
                    value = _parcJSONValue_StringParser(parser);
                } else if (peek == '{') {
                    value = parcJSONValue_ObjectParser(parser);
                } else if (peek == '[') {
                    value = _parcJSONValue_ArrayParser(parser);
                } else {
                    value = _parcJSONValue_NumberParser(parser);
                }

                if (value != NULL) {
                    parcJSONArray_AddValue(array, value);
                    parcJSONValue_Release(&value);
                } else {
                    parcJSONArray_Release(&array);
                    break;
                }
            }
        }
    }

    return result;
}

static void
_displayBoolean(const PARCJSONValue *value, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, ".value=%s", value->value.boolean == true ? "true" : "false");
}

static void
_displayNumber(const PARCJSONValue *value, int indentation)
{
    if (value->value.number.internalDoubleRepresentation) {
        parcDisplayIndented_PrintLine(indentation, ".value=%Lf", value->value.number.internalDoubleValue);
    } else {
        parcDisplayIndented_PrintLine(indentation,
                                      ".value.number={ sign=%d whole=%lld fractionLog10=%d fraction=%lld exponent=%lld",
                                      value->value.number.sign,
                                      value->value.number.whole,
                                      (int) value->value.number.fractionLog10,
                                      value->value.number.fraction,
                                      value->value.number.exponent);
    }
}

void
parcJSONValue_AssertValid(const PARCJSONValue *value)
{
    assertNotNull(value, "PARCJSONValue cannot be NULL.");
}

bool
parcJSONValue_IsValid(const PARCJSONValue *value)
{
    bool result = true;

    if (value == NULL) {
        result = false;
    }

    return result;
}

bool
parcJSONValue_IsNull(const PARCJSONValue *value)
{
    parcJSONValue_OptionalAssertValid(value);

    return (value->type == PARCJSONValueType_Null);
}

bool
parcJSONValue_IsBoolean(const PARCJSONValue *value)
{
    parcJSONValue_OptionalAssertValid(value);

    return (value->type == PARCJSONValueType_Boolean);
}

bool
parcJSONValue_IsNumber(const PARCJSONValue *value)
{
    parcJSONValue_OptionalAssertValid(value);

    return (value->type == PARCJSONValueType_Number);
}

bool
parcJSONValue_IsJSON(const PARCJSONValue *value)
{
    parcJSONValue_OptionalAssertValid(value);

    return (value->type == PARCJSONValueType_JSON);
}

bool
parcJSONValue_IsString(const PARCJSONValue *value)
{
    parcJSONValue_OptionalAssertValid(value);

    return (value->type == PARCJSONValueType_String);
}

bool
parcJSONValue_IsArray(const PARCJSONValue *value)
{
    parcJSONValue_OptionalAssertValid(value);

    return (value->type == PARCJSONValueType_Array);
}

PARCJSONValue *
parcJSONValue_CreateFromNULL(void)
{
    // Strictly speaking, this could just be a singleton, rather than allocated every time.

    PARCJSONValue *result = _createValue(PARCJSONValueType_Null);

    return result;
}

PARCJSONValue *
parcJSONValue_CreateFromBoolean(bool value)
{
    PARCJSONValue *result = _createValue(PARCJSONValueType_Boolean);
    if (result != NULL) {
        result->value.boolean = value;
    }
    return result;
}

PARCJSONValue *
parcJSONValue_CreateFromFloat(long double value)
{
    PARCJSONValue *result = _parcJSONValue_CreateNumber(0, 0, 0, 0, 0);
    result->value.number.internalDoubleRepresentation = true;
    result->value.number.internalDoubleValue = value;
    return result;
}

PARCJSONValue *
parcJSONValue_CreateFromInteger(int64_t value)
{
    PARCJSONValue *result = _parcJSONValue_CreateNumber(1, value, 0, 0, 0);
    return result;
}

PARCJSONValue *
parcJSONValue_CreateFromString(PARCBuffer *value)
{
    parcBuffer_OptionalAssertValid(value);

    PARCJSONValue *result = _createValue(PARCJSONValueType_String);
    if (result != NULL) {
        result->value.string = parcBuffer_Acquire(value);
    }
    return result;
}

PARCJSONValue *
parcJSONValue_CreateFromCString(const char *value)
{
    assertNotNull(value, "String cannot be NULL.");

    PARCJSONValue *result = _createValue(PARCJSONValueType_String);
    if (result != NULL) {
        result->value.string = parcBuffer_AllocateCString(value);
    }
    return result;
}

PARCJSONValue *
parcJSONValue_CreateFromJSONArray(PARCJSONArray *value)
{
    PARCJSONValue *result = _createValue(PARCJSONValueType_Array);
    if (result != NULL) {
        result->value.array = parcJSONArray_Acquire(value);
    }
    return result;
}

PARCJSONValue *
parcJSONValue_CreateFromJSON(PARCJSON *value)
{
    PARCJSONValue *result = _createValue(PARCJSONValueType_JSON);
    if (result != NULL) {
        result->value.object = parcJSON_Acquire(value);
    }
    return result;
}

PARCJSONValue *
parcJSONValue_CreateFromTimeval(const struct timeval *timeval)
{
    PARCJSON *jsonTimeval = parcJSON_Create();
    parcJSON_AddInteger(jsonTimeval, "seconds", timeval->tv_sec);
    parcJSON_AddInteger(jsonTimeval, "micros", timeval->tv_usec);

    PARCJSONValue *result = _createValue(PARCJSONValueType_JSON);
    if (result != NULL) {
        result->value.object = jsonTimeval;
    }

    return result;
}

PARCJSONValue *
parcJSONValue_CreateFromTimespec(const struct timespec *timespec)
{
    PARCJSON *jsonTimespec = parcJSON_Create();
    parcJSON_AddInteger(jsonTimespec, "seconds", timespec->tv_sec);
    parcJSON_AddInteger(jsonTimespec, "nanos", timespec->tv_nsec);

    PARCJSONValue *result = _createValue(PARCJSONValueType_JSON);
    if (result != NULL) {
        result->value.object = jsonTimespec;
    }

    return result;
}

void
parcJSONValue_Display(const PARCJSONValue *value, int indentation)
{
    parcDisplayIndented_PrintLine(indentation, "PARCJSONValue@%p {", value);
    if (value != NULL) {
        parcDisplayIndented_PrintLine(indentation + 1, ".type=%d", value->type);

        switch (value->type) {
            case PARCJSONValueType_Boolean:
                _displayBoolean(value, indentation + 1);
                break;
            case PARCJSONValueType_String:
                parcBuffer_Display(value->value.string, indentation + 1);
                break;
            case PARCJSONValueType_Number:
                _displayNumber(value, indentation + 1);
                break;
            case PARCJSONValueType_Array:
                parcJSONArray_Display(value->value.array, indentation + 1);
                break;
            case PARCJSONValueType_JSON:
                parcJSON_Display(value->value.object, indentation + 1);
                break;
            case PARCJSONValueType_Null:
                parcDisplayIndented_PrintLine(indentation + 1, ".value=null");
                break;
            default:
                trapIllegalValue(value->type, "Unknown PARCJSONValue type %d", value->type);
        }
    }
    parcDisplayIndented_PrintLine(indentation, "}");
}

parcObject_ImplementAcquire(parcJSONValue, PARCJSONValue);

parcObject_ImplementRelease(parcJSONValue, PARCJSONValue);

static bool
_equalsNumber(const PARCJSONValue *valueA, const PARCJSONValue *valueB)
{
    bool result = false;

    if (valueA->value.number.internalDoubleRepresentation) {
        if (valueB->value.number.internalDoubleRepresentation) {
            if (valueA->value.number.internalDoubleValue == valueB->value.number.internalDoubleValue) {
                result = true;
            }
        }
    } else {
        if (valueA->value.number.sign == valueB->value.number.sign) {
            if (valueA->value.number.whole == valueB->value.number.whole) {
                if (valueA->value.number.fraction == valueB->value.number.fraction) {
                    if (valueA->value.number.exponent == valueB->value.number.exponent) {
                        result = true;
                    }
                }
            }
        }
    }

    return result;
}

bool
parcJSONValue_Equals(const PARCJSONValue *objA, const PARCJSONValue *objB)
{
    bool result = false;

    if (objA == NULL || objB == NULL) {
        result = (objA == objB);
    } else {
        if (objA->type == objB->type) {
            switch (objA->type) {
                case PARCJSONValueType_Boolean:
                    result = objA->value.boolean == objB->value.boolean;
                    break;
                case PARCJSONValueType_String:
                    result = parcBuffer_Equals(objA->value.string, objB->value.string);
                    break;
                case PARCJSONValueType_Number:
                    result = _equalsNumber(objA, objB);
                    break;
                case PARCJSONValueType_Array:
                    result = parcJSONArray_Equals(objA->value.array, objB->value.array);
                    break;
                case PARCJSONValueType_JSON:
                    result = parcJSON_Equals(objA->value.object, objB->value.object);
                    break;
                case PARCJSONValueType_Null:
                    result = true;
                    break;
            }
        }
    }

    return result;
}

PARCJSONArray *
parcJSONValue_GetArray(const PARCJSONValue *value)
{
    parcJSONValue_OptionalAssertValid(value);

    trapUnexpectedStateIf(!parcJSONValue_IsArray(value), "Expected type to be array, actual type %d", value->type);

    return value->value.array;
}

bool
parcJSONValue_GetBoolean(const PARCJSONValue *value)
{
    parcJSONValue_OptionalAssertValid(value);

    trapUnexpectedStateIf(!parcJSONValue_IsBoolean(value), "Expected type to be boolean, actual type %d", value->type);

    return value->value.boolean;
}

static long double
_parcJSONValue_GetNumber(const PARCJSONValue *value)
{
    long double fraction = value->value.number.fraction / powl(10.0, value->value.number.fractionLog10);
    long double number = (long double) value->value.number.sign * ((long double) value->value.number.whole + fraction);

    long double result = number * powl(10.0, (long double) value->value.number.exponent);

    return result;
}

long double
parcJSONValue_GetFloat(const PARCJSONValue *value)
{
    parcJSONValue_OptionalAssertValid(value);

    long double result = 0;

    if (value->value.number.internalDoubleRepresentation) {
        result = value->value.number.internalDoubleValue;
    } else {
        result = _parcJSONValue_GetNumber(value);
    }

    return result;
}

int64_t
parcJSONValue_GetInteger(const PARCJSONValue *value)
{
    int64_t result = llrint(_parcJSONValue_GetNumber(value));
    return result;
}

PARCBuffer *
parcJSONValue_GetString(const PARCJSONValue *value)
{
    parcJSONValue_OptionalAssertValid(value);

    trapUnexpectedStateIf(!parcJSONValue_IsString(value), "Expected type to be string, actual type %d", value->type);

    return value->value.string;
}

PARCJSON *
parcJSONValue_GetJSON(const PARCJSONValue *value)
{
    parcJSONValue_OptionalAssertValid(value);

    trapUnexpectedStateIf(!parcJSONValue_IsJSON(value), "Expected type to be string, actual type %d", value->type);

    return value->value.object;
}

struct timeval *
parcJSONValue_GetTimeval(const PARCJSONValue *jsonTimeval, struct timeval *timeval)
{
    assertNotNull(jsonTimeval, "Parameter jsonTimeval must be a non-null PARCJSON pointer.");

    PARCJSON *json = parcJSONValue_GetJSON(jsonTimeval);
    PARCJSONValue *value = parcJSON_GetValueByName(json, "seconds");
    timeval->tv_sec = parcJSONValue_GetInteger(value);
    value = parcJSON_GetValueByName(json, "micros");
    timeval->tv_usec = (int) parcJSONValue_GetInteger(value);

    return timeval;
}

struct timespec *
parcJSONValue_GetTimespec(const PARCJSONValue *jsonTimespec, struct timespec *timespec)
{
    assertNotNull(jsonTimespec, "Parameter jsonTimeval must be a non-null PARCJSON pointer.");

    PARCJSON *json = parcJSONValue_GetJSON(jsonTimespec);
    PARCJSONValue *value = parcJSON_GetValueByName(json, "seconds");
    timespec->tv_sec = parcJSONValue_GetInteger(value);
    value = parcJSON_GetValueByName(json, "nanos");
    timespec->tv_nsec = (int) parcJSONValue_GetInteger(value);

    return timespec;
}

static PARCBufferComposer *
_buildStringNumber(const PARCJSONValue *value, PARCBufferComposer *string)
{
    if (value->value.number.internalDoubleRepresentation) {
        parcBufferComposer_Format(string, "%Lf", value->value.number.internalDoubleValue);
    } else {
        parcBufferComposer_Format(string, "%s%" PRId64,
                                  value->value.number.sign == -1 ? "-" : "",
                                  value->value.number.whole);
        if (value->value.number.fraction > 0) {
            parcBufferComposer_Format(string, ".%0*" PRId64,
                                      (int) value->value.number.fractionLog10,
                                      value->value.number.fraction);
        }

        if (value->value.number.exponent != 0) {
            parcBufferComposer_Format(string, "e%" PRId64,
                                      value->value.number.exponent);
        }
    }
    return string;
}

static PARCBufferComposer *
_buildStringString(const PARCJSONValue *value, PARCBufferComposer *composer, bool compact)
{
    parcBufferComposer_PutChar(composer, '"');

    while (parcBuffer_Remaining(value->value.string)) {
        uint8_t c = parcBuffer_GetUint8(value->value.string);
        if (c == '"') {
            parcBufferComposer_PutString(composer, "\\\"");
        } else if (c == '\b') {
            parcBufferComposer_PutString(composer, "\\b");
        } else if (c == '\f') {
            parcBufferComposer_PutString(composer, "\\f");
        } else if (c == '\n') {
            parcBufferComposer_PutString(composer, "\\n");
        } else if (c == '\r') {
            parcBufferComposer_PutString(composer, "\\r");
        } else if (c == '\t') {
            parcBufferComposer_PutString(composer, "\\t");
        } else if ((c == '/') && !compact) {
            parcBufferComposer_PutString(composer, "\\/");
        } else if (c == '\\') {
            parcBufferComposer_PutString(composer, "\\\\");
        } else {
            parcBufferComposer_PutChar(composer, c);
        }
    }

    parcBuffer_Rewind(value->value.string);
    parcBufferComposer_PutChar(composer, '"');

    return composer;
}

PARCBufferComposer *
parcJSONValue_BuildString(const PARCJSONValue *value, PARCBufferComposer *composer, bool compact)
{
    parcJSONValue_OptionalAssertValid(value);

    if (value->type == PARCJSONValueType_Boolean) {
        parcBufferComposer_PutString(composer, value->value.boolean ? "true" : "false");
    } else if (value->type == PARCJSONValueType_String) {
        _buildStringString(value, composer, compact);
    } else if (value->type == PARCJSONValueType_Number) {
        _buildStringNumber(value, composer);
    } else if (value->type == PARCJSONValueType_Array) {
        parcJSONArray_BuildString(value->value.array, composer, compact);
    } else if (value->type == PARCJSONValueType_JSON) {
        parcJSON_BuildString(value->value.object, composer, compact);
    } else if (value->type == PARCJSONValueType_Null) {
        parcBufferComposer_PutString(composer, "null");
    } else {
        trapIllegalValue(value->type, "Unknown value type: %d", value->type);
    }

    return composer;
}

static char *
_parcJSONValue_ToString(const PARCJSONValue *value, bool compact)
{
    PARCBufferComposer *composer = parcBufferComposer_Create();
    parcJSONValue_BuildString(value, composer, compact);

    PARCBuffer *tempBuffer = parcBufferComposer_ProduceBuffer(composer);
    char *result = parcBuffer_ToString(tempBuffer);
    parcBuffer_Release(&tempBuffer);

    parcBufferComposer_Release(&composer);

    return result;
}

char *
parcJSONValue_ToString(const PARCJSONValue *value)
{
    return _parcJSONValue_ToString(value, false);
}

char *
parcJSONValue_ToCompactString(const PARCJSONValue *value)
{
    return _parcJSONValue_ToString(value, true);
}

PARCJSONValue *
parcJSONValue_Parser(PARCJSONParser *parser)
{
    char nextCharacter = parcJSONParser_PeekNextChar(parser);
    switch (nextCharacter) {
        case ',':
            break;

        case ']':
            return NULL;

        case 'n':
            return _parcJSONValue_NullParser(parser);

        case 't':
            return _parcJSONValue_TrueParser(parser);

        case 'f':
            return _parcJSONValue_FalseParser(parser);

        case '"':
            return _parcJSONValue_StringParser(parser);

        case '[':
            return _parcJSONValue_ArrayParser(parser);

        case '{':
            return parcJSONValue_ObjectParser(parser);

        default:
            return _parcJSONValue_NumberParser(parser);
    }

    return NULL;
}

PARCJSONValue *
parcJSONValue_ObjectParser(PARCJSONParser *parser)
{
    PARCJSONValue *result = NULL;

    // absorb the (required) '{' character.
    if (parcJSONParser_NextChar(parser) == '{') {
        PARCJSON *json = parcJSON_Create();

        while (parcJSONParser_Remaining(parser)) {
            char c = parcJSONParser_PeekNextChar(parser);
            if (c == '}') {
                // Absorb the '}' and terminate.
                parcJSONParser_NextChar(parser);
                result = parcJSONValue_CreateFromJSON(json);
                break;
            } else if (c == ',') {
                // absorb the ',' character and continue
                parcJSONParser_NextChar(parser);
            } else if (c == '"') {
                PARCJSONPair *pair = parcJSONPair_Parser(parser);
                if (pair == NULL) {
                    break;
                }
                parcJSON_AddPair(json, pair);
                parcJSONPair_Release(&pair);
            } else {
                break;
            }
        }
        parcJSON_Release(&json);
    }

    return result;
}

