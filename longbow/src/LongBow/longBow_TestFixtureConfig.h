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
 * @file longBow_TestFixtureConfig.h
 * @ingroup testing
 * @brief The Test Fixture configuration interface
 *
 */
#ifndef LongBow_longBow_FixtureConfig_h
#define LongBow_longBow_FixtureConfig_h
#include <stdbool.h>

struct longbow_fixture_config {
    char *name;

    bool enabled;
};
typedef struct longbow_fixture_config LongBowTestFixtureConfig;

/**
 * Create and initialise a LongBowTestFixtureConfig instance.
 *
 * @param [in] name The name of the Test Fixture
 * @param [in] enabled True if the fixture is enabled, false otherwise.
 *
 * @return non-NULL A pointer to an allocated `LongBowTestFixtureConfig` isntance that must be destroyed via `longBowTestFixtureConfig_Destroy`.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 *
 * @see longBowTestFixtureConfig_Destroy
 */
LongBowTestFixtureConfig *longBowTestFixtureConfig_Create(const char *name, bool enabled);

/**
 * Destroy a LongBowTestFixtureConfig intance.
 *
 * @param [in,out] configPtr A pointer to a pointer to a `LongBowTestFixtureConfig` instance.
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 *
 * @see longBowTestFixtureConfig_Create
 */
void longBowTestFixtureConfig_Destroy(LongBowTestFixtureConfig **configPtr);
#endif // LongBow_longBow_FixtureConfig_h
