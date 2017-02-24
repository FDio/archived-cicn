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
 * @file longBow_MeasureTime.h
 *
 * Measure elapsed time, providing various fetching and reporting mechanisms.
 *
 */
#ifndef __LongBow__longBow_Measure__
#define __LongBow__longBow_Measure__
#include <stdint.h>
#include <stdbool.h>

#ifdef LongBow_DISABLE_MEASUREMENTS
#  define longBowMeasureTime(_iterations_)
#else
#  define longBowMeasureTime(_iterations_) for (LongBowMeasureTime *_measure = longBowMeasureTime_Start(_iterations_); \
                                                longBowMeasureTime_CountDown(_measure) ? true : (longBowMeasureTime_Report(_measure, __FILE__, __func__, __LINE__), longBowMeasureTime_Destroy(&_measure), false); )

#endif

struct longBowMeasureTime;
typedef struct longBowMeasureTime LongBowMeasureTime;

/**
 * Create and start a `LongBowMeasureTime` instance.
 *
 * @param [in] iterations The number of iterations to perform when used with `longBowMeasureTime_CountDown`
 *
 * @return non-NULL A pointer to a valid LongBowMeasureTime instance that must be destroyed by `longBowMeasureTime_Destroy`
 */
LongBowMeasureTime *longBowMeasureTime_Start(unsigned int iterations);

/**
 * Report on the `LongBowMeasureTime` instance.
 *
 * @param [in] measure A pointer to a valid LongBowMeasureTime instance.
 * @param [in] file A pointer to a nul-terminated C string representing the file name causing the report.
 * @param [in] function A pointer to a nul-terminated C string representing the function name causing the report.
 * @param [in] line An unsigned integer representing the line number of the file causing the report.
 *
 * @return true The report was successful.
 */
bool longBowMeasureTime_Report(LongBowMeasureTime *measure, const char *file, const char *function, unsigned int line);

/**
 * A simple count-down supporting measurement iterations.
 *
 * See {@link longBowMeasureTime} for an example.
 *
 * @param [in] measure A pointer to a valid LongBowMeasureTime instance.
 *
 * @return The current value of the counter.
 */
unsigned int longBowMeasureTime_CountDown(LongBowMeasureTime *measure);

/**
 * Get the total number of microseconds represented by the `LongBowMeasureTime` instance.
 *
 * @param [in] measure A pointer to a valid LongBowMeasureTime instance.
 *
 * @return The number of microseconds represented by the `LongBowMeasureTime` instance.
 */
uint64_t longBowMeasureTime_GetMicroseconds(const LongBowMeasureTime *measure);

/**
 * Get the total number of nanoseconds represented by the `LongBowMeasureTime` instance.
 *
 * @param [in] measure A pointer to a valid LongBowMeasureTime instance.
 *
 * @return The number of nanoseconds represented by the `LongBowMeasureTime` instance.
 */
uint64_t longBowMeasureTime_GetNanoseconds(const LongBowMeasureTime *measure);

/**
 * Destroy a valid `LongBowMeasureTime` instance.
 *
 * @param [in,out] instancePtr A pointer to a pointer to a valid LongBowMeasureTime instance, that will be set to zero.
 */
void longBowMeasureTime_Destroy(LongBowMeasureTime **instancePtr);

#endif /* defined(__LongBow__longBow_Measure__) */
