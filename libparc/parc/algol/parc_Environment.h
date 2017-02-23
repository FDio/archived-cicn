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
 * @file parc_Environment.h
 * @ingroup inputoutput
 * @brief Functions to access and manipulate the runtime environment.
 *
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
#ifndef libparc_parc_Environment_h
#define libparc_parc_Environment_h

/**
 *
 * Get the current home directory for the running process.
 *
 * @return A C string containing the name of the home directory.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
const char *parcEnvironment_GetHomeDirectory(void);
#endif // libparc_parc_Environment_h
