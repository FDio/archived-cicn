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

#include <parc/statistics/parc_BasicStats.h>

struct PARCBasicStats {
    int64_t count;
    double maximum;
    double minimum;
    double mean;
    double variance;
};

static inline bool
_parcBasicStats_FloatEquals(double x, double y, double e)
{
    return fabs(x - y) < e;
}

static bool
_parcBasicStats_Destructor(PARCBasicStats **instancePtr)
{
    parcAssertNotNull(instancePtr, "Parameter must be a non-null pointer to a PARCBasicStats pointer.");

    return true;
}

parcObject_ImplementAcquire(parcBasicStats, PARCBasicStats);

parcObject_ImplementRelease(parcBasicStats, PARCBasicStats);

parcObject_Override(
    PARCBasicStats, PARCObject,
    .destructor = (PARCObjectDestructor *) _parcBasicStats_Destructor,
    .copy = (PARCObjectCopy *) parcBasicStats_Copy,
    .toString = (PARCObjectToString *)  parcBasicStats_ToString,
    .equals = (PARCObjectEquals *)  parcBasicStats_Equals,
    .compare = (PARCObjectCompare *)  parcBasicStats_Compare,
    .hashCode = (PARCObjectHashCode *)  parcBasicStats_HashCode,
    .toJSON = (PARCObjectToJSON *)  parcBasicStats_ToJSON);


void
parcBasicStats_AssertValid(const PARCBasicStats *instance)
{
    parcAssertTrue(parcBasicStats_IsValid(instance),
               "PARCBasicStats is not valid.");
}


PARCBasicStats *
parcBasicStats_Create(void)
{
    PARCBasicStats *result = parcObject_CreateInstance(PARCBasicStats);

    if (result != NULL) {
        result->count = 0;
        result->mean = 0;
        result->variance = 0;
        result->maximum = 0;
        result->minimum = 0;
    }

    return result;
}

int
parcBasicStats_Compare(const PARCBasicStats *instance, const PARCBasicStats *other)
{
    int result = 0;

    return result;
}

PARCBasicStats *
parcBasicStats_Copy(const PARCBasicStats *original)
{
    PARCBasicStats *result = parcBasicStats_Create();
    result->count = original->count;
    result->mean = original->mean;
    result->variance = original->variance;
    result->maximum = original->maximum;
    result->minimum = original->minimum;

    return result;
}

void
parcBasicStats_Display(const PARCBasicStats *stats, int indentation)
{
    parcDisplayIndented_PrintLine(indentation,
                                  "PARCBasicStats@%p { .count=%" PRId64 " .minimum=%llf .maximum=%llf .mean=%llf }",
                                  stats, stats->count, stats->minimum, stats->maximum, stats->mean);
}

bool
parcBasicStats_Equals(const PARCBasicStats *x, const PARCBasicStats *y)
{
    bool result = false;

    if (x == y) {
        result = true;
    } else if (x == NULL || y == NULL) {
        result = false;
    } else {
        if (x->count == y->count) {
            if (_parcBasicStats_FloatEquals(x->maximum, y->maximum, 0.00001)) {
                if (_parcBasicStats_FloatEquals(x->minimum, y->minimum, 0.00001)) {
                    if (_parcBasicStats_FloatEquals(x->mean, y->mean, 0.00001)) {
                        result = true;
                    }
                }
            }
        }
    }

    return result;
}

PARCHashCode
parcBasicStats_HashCode(const PARCBasicStats *instance)
{
    PARCHashCode result = 0;

    return result;
}

bool
parcBasicStats_IsValid(const PARCBasicStats *stats)
{
    bool result = false;

    if (stats != NULL) {
        result = true;
    }

    return result;
}

PARCJSON *
parcBasicStats_ToJSON(const PARCBasicStats *stats)
{
    PARCJSON *result = parcJSON_Create();

    if (result != NULL) {
        PARCJSONPair *pair = parcJSONPair_CreateFromDouble("maximum", stats->maximum);
        parcJSON_AddPair(result, pair);
        parcJSONPair_Release(&pair);

        pair = parcJSONPair_CreateFromDouble("minimum", stats->minimum);
        parcJSON_AddPair(result, pair);
        parcJSONPair_Release(&pair);

        pair = parcJSONPair_CreateFromDouble("mean", stats->mean);
        parcJSON_AddPair(result, pair);
        parcJSONPair_Release(&pair);

        pair = parcJSONPair_CreateFromDouble("variance", stats->variance);
        parcJSON_AddPair(result, pair);
        parcJSONPair_Release(&pair);

        pair = parcJSONPair_CreateFromInteger("count", stats->count);
        parcJSON_AddPair(result, pair);
        parcJSONPair_Release(&pair);
    }

    return result;
}

char *
parcBasicStats_ToString(const PARCBasicStats *stats)
{
    char *result = parcMemory_Format("PARCBasicStats@%p { .count=%" PRId64 " .minimum=%llf .maximum=%llf .mean=%llf }",
                                     stats, stats->count, stats->minimum, stats->maximum, stats->mean);

    return result;
}

void
parcBasicStats_Update(PARCBasicStats *stats, double value)
{
    stats->count++;

    if (value > stats->maximum) {
        stats->maximum = value;
    }

    if (value < stats->minimum) {
        stats->minimum = value;
    }

    double mean_ = stats->mean;

    double xMinusOldMean = value - mean_;

    stats->mean = mean_ + xMinusOldMean / stats->count;

    double xMinusCurrentMean = value - stats->mean;

    stats->variance = ((stats->variance * (stats->count - 1)) + xMinusOldMean * xMinusCurrentMean) / stats->count;
}

double
parcBasicStats_Mean(const PARCBasicStats *stats)
{
    return stats->mean;
}

double
parcBasicStats_Variance(const PARCBasicStats *stats)
{
    return stats->variance;
}

double
parcBasicStats_StandardDeviation(const PARCBasicStats *stats)
{
    return sqrt(stats->variance);
}

double
parcBasicStats_Maximum(const PARCBasicStats *stats)
{
    return stats->maximum;
}

double
parcBasicStats_Minimum(const PARCBasicStats *stats)
{
    return stats->minimum;
}

double
parcBasicStats_Range(const PARCBasicStats *stats)
{
    return stats->maximum - stats->minimum;
}
