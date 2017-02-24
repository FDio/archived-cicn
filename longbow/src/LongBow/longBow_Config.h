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
 * @file longBow_Config.h
 * @ingroup internals
 * @brief Support for LongBow Configuration
 *
 */
#ifndef LONGBOWCONFIGURATION_H_
#define LONGBOWCONFIGURATION_H_
#include <stdbool.h>
#include <stdint.h>

struct longbow_configuration;

/**
 * @typedef LongBowConfig
 * @brief The LongBow Configuration
 */
typedef struct longbow_configuration LongBowConfig;

/**
 * Destroy a LongBowConfig instance.
 *
 * @param [in,out] configPtr A pointer to a LongBowConfig instance pointer.
 */
void longBowConfig_Destroy(LongBowConfig **configPtr);

/**
 * Create a LongBowConfig structure instance from the given array of parameters.
 *
 * The function parses argv style arguments and composes a LongBowConfig structure.
 *
 * The argv-style arguments may include parameters not related to creating a LongBowConfig structure.
 *
 * For example, the arguments may only be `--help` and prints a help message but doesn't create a `LongBowConfig` structure.
 *
 * @param [in] argc The number of elements in the `argv` array.
 * @param [in] argv An array of nul-terminated C strings.
 * @param [in] mainFileName The string printed for the `--main` option.
 *
 * @return non-NULL A LongBowConfig structure instance suitable for running a test
 * @return NULL Nothing suitable for running a test (not an error).
 */
LongBowConfig *longBowConfig_Create(int argc, char *argv[], const char *mainFileName);

/**
 * Get a property stored in the configuration.
 * @param [in] config A pointer to a valid LongBowConfig instance.
 * @param [in] format A pointer to a valid, nul-terminated format string.
 *
 * @return non-NULL A pointer to nul-terminated, C string.
 */
const char *longBowConfig_GetProperty(const LongBowConfig *config, const char *format, ...);

/**
 * Set the property @p name to @p value
 *
 * @param [in] config A pointer to a valid LongBowConfig instance.
 *
 * @return true
 * @return true
 */
bool longBowConfig_SetProperty(const LongBowConfig *config, const char *name, const char *value);

/**
 * Get the property @p name interpreted as a 32-bit integer.
 *
 * @param [in] config A pointer to a valid LongBowConfig instance.
 *
 * @return The property @p name interpreted as a 32-bit integer.
 */
uint32_t longBowConfig_GetUint32(const LongBowConfig *config, uint32_t defaultValue, const char *format, ...);

/**
 * Get the value of the configuration property named by the formatted property name.
 * If the given LongBowConfig instance is not available, or the property is not present, the default value is returned.
 *
 * @param [in] config A pointer to a valid LongBowConfig instance.
 * @param [in] defaultValue Either true or false.
 * @param [in] format A printf format string followed by appropriate parameters used to construct the property name.
 *
 * @return true The property was present with the value true, or the property was not present and the default value is true.
 * @return false The property was present with the value false, or the property was not present and the default value is false.
 */
bool longBowConfig_GetBoolean(const LongBowConfig *config, bool defaultValue, const char *format, ...);

/**
 * Return `true` if the given `LongBowConfig` instance specifies that test cases are to be run in a sub-process.
 *
 * @param [in] config A pointer to a valid LongBowConfig instance.
 * @return true if the given specification stipulates tests are to run in a sub-process.
 */
bool longBowConfig_IsRunForked(const LongBowConfig *config);

/**
 * Indicate if the given configuration is specifying the 'trace' flag.
 *
 * @param [in] config A pointer to a valid LongBowConfig instance.
 *
 * @return true The given configuration is specifying the 'trace' flag.
 * @return false The given configuration is not specifying the 'trace' flag.
 *
 * Example:
 * @code
 * if (longBowConfig_IsTrace(longBowRunner_GetConfiguration(testRunner)) {
 *     longBowTrace_Report(testRunner->configuration);
 * }
 * @endcode
 */
bool longBowConfig_IsTrace(const LongBowConfig *config);

#endif // LONGBOWCONFIGURATION_H_
