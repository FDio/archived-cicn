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
 * @file debugging.h
 * @brief Debugging Utilities
 *
 */
#ifndef LongBow_debugging_h
#define LongBow_debugging_h

#include <LongBow/longBow_Debug.h>

#include <LongBow/longBow_MeasureTime.h>

/**
 * @def longBow_function
 * @brief The compile time function name.
 */
#if __STDC_VERSION__ >= 199901L
#    define longBow_function __func__
#else
#    define longBow_function ((const char *) 0)
#endif

/**
 * @def longBowDebug
 * @brief Print a debugging message.
 *
 * @param ... A printf-style format string followed by a variable number of parameters supplying values for the format string.
 */
#ifndef LONGBOW_DEBUG_DISABLED
#   define longBowDebug(...) \
    do { longBowDebug_Message(NULL, longBowLocation_Create(__FILE__, longBow_function, __LINE__), __VA_ARGS__); } while (0)
#else
#   define longBowDebug(...) \
    do { } while (0)
#endif
#endif
