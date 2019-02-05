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

#include <math.h>

#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_DisplayIndented.h>
#include <parc/algol/parc_Memory.h>

#include <parc/statistics/parc_EWMA.h>

struct PARCEWMA {
    bool initialized;
    int64_t value;
    double coefficient;
    double coefficient_r;
};

static inline bool
_parcEWMA_FloatEquals(double x, double y, double e)
{
    return fabs(x - y) < e;
}

static bool
_parcEWMA_Destructor(PARCEWMA **instancePtr)
{
    parcAssertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCEWMA pointer.");

    return true;
}

parcObject_ImplementAcquire(parcEWMA, PARCEWMA);

parcObject_ImplementRelease(parcEWMA, PARCEWMA);

parcObject_Override(
    PARCEWMA, PARCObject,
    .destructor = (PARCObjectDestructor *) _parcEWMA_Destructor,
    .copy = (PARCObjectCopy *) parcEWMA_Copy,
    .toString = (PARCObjectToString *)  parcEWMA_ToString,
    .equals = (PARCObjectEquals *)  parcEWMA_Equals,
    .compare = (PARCObjectCompare *)  parcEWMA_Compare,
    .hashCode = (PARCObjectHashCode *)  parcEWMA_HashCode,
    .toJSON = (PARCObjectToJSON *)  parcEWMA_ToJSON);

void
parcEWMA_AssertValid(const PARCEWMA *instance)
{
    parcAssertTrue(parcEWMA_IsValid(instance),
               "PARCEWMA is not valid.");
}

PARCEWMA *
parcEWMA_Create(double coefficient)
{
    PARCEWMA *result = parcObject_CreateInstance(PARCEWMA);
    if (result != NULL) {
        result->initialized = false;
        result->value = 0;
        result->coefficient = coefficient;
        result->coefficient_r = 1.0 - coefficient;
    }

    return result;
}

int
parcEWMA_Compare(const PARCEWMA *instance, const PARCEWMA *other)
{
    int result = 0;

    if (instance == other) {
        result = 0;
    } else if (instance == NULL) {
        result = -1;
    } else if (other == NULL) {
        result = 1;
    } else {
        result = (int)(instance->value - other->value);
    }

    return result;
}

PARCEWMA *
parcEWMA_Copy(const PARCEWMA *original)
{
    PARCEWMA *result = parcEWMA_Create(original->coefficient);
    result->initialized = original->initialized;
    result->value = original->value;

    return result;
}

void
parcEWMA_Display(const PARCEWMA *ewma, int indentation)
{
    parcDisplayIndented_PrintLine(indentation,
                                  "PARCEWMA@%p { .initialized=%s .coefficient=%lf, .value=%" PRId64 " }",
                                  ewma,
                                  ewma->initialized ? "true" : "false",
                                  ewma->coefficient,
                                  ewma->value);
}

bool
parcEWMA_Equals(const PARCEWMA *x, const PARCEWMA *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        if (x->initialized == y->initialized) {
            if (_parcEWMA_FloatEquals(x->coefficient, y->coefficient, 0.00001)) {
                if (_parcEWMA_FloatEquals((double)(x->value), (double)(y->value), 0.00001)) {
                    result = true;
                }
            }
        }
    }

    return result;
}

PARCHashCode
parcEWMA_HashCode(const PARCEWMA *instance)
{
    PARCHashCode result = 0;

    return result;
}

bool
parcEWMA_IsValid(const PARCEWMA *instance)
{
    bool result = false;

    if (instance != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
parcEWMA_ToJSON(const PARCEWMA *instance)
{
    PARCJSON *result = parcJSON_Create();

    if (result != NULL) {
    }

    return result;
}

char *
parcEWMA_ToString(const PARCEWMA *ewma)
{
    char *result = parcMemory_Format("PARCEWMA@%p { .initialized=%s .coefficient=%lf, .value=%" PRId64 " }",
                                     ewma,
                                     ewma->initialized ? "true" : "false",
                                     ewma->coefficient,
                                     ewma->value);
    return result;
}

int64_t
parcEWMA_Update(PARCEWMA *ewma, const int64_t value)
{
    if (ewma->initialized) {
        // E_t = a * V + (1 - a) * E_(t-1)
        double x = (ewma->coefficient * value);
        double y = (ewma->coefficient_r * ewma->value);

        ewma->value = (int64_t)(x + y);
    } else {
        ewma->value = value;
        ewma->initialized = true;
    }
    return ewma->value;
}

int64_t
parcEWMA_GetValue(const PARCEWMA *ewma)
{
    return ewma->value;
}
