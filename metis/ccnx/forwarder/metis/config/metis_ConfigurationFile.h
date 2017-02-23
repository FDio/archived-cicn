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
 * @file metis_ConfigurationFile.h
 * @brief Accepts a filename and provides a means to read it into MetisConfiguration
 *
 * Reads a configuration file and converts the lines in to configuration commands for use
 * in MetisConfiguration.
 *
 * Accepts '#' lines as comments.  Skips blank and whitespace-only lines.
 *
 */

#ifndef Metis__metis_ConfigurationFile_h
#define Metis__metis_ConfigurationFile_h

#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

struct metis_configuration_file;
typedef struct metis_configuration_file MetisConfigurationFile;

/**
 * Creates a MetisConfigurationFile to prepare to process the file
 *
 * Prepares the object and opens the file.  Makes sure we can read the file.
 * Does not read the file or process any commands from the file.
 *
 * @param [in] metis An allocated MetisForwarder to configure with the file
 * @param [in] filename The file to use
 *
 * @retval non-null An allocated MetisConfigurationFile that is readable
 * @retval null An error
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
MetisConfigurationFile *metisConfigurationFile_Create(MetisForwarder *metis, const char *filename);

/**
 * Reads the configuration file line-by-line and issues commands to MetisConfiguration
 *
 * Reads the file line by line.  Skips '#' and blank lines.  Creates CPI objects from the
 * lines and feeds them to MetisConfiguration.
 *
 * Will stop on the first error.  Lines already processed will not be un-done.
 *
 * @param [in] configFile An allocated MetisConfigurationFile
 *
 * @retval true The entire files was processed without error.
 * @retval false There was an error in the file.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool metisConfigurationFile_Process(MetisConfigurationFile *configFile);

//void metisConfigurationFile_ProcessForwardingStrategies(MetisConfiguration * config,  MetisConfigurationFile * configFile);

/**
 * Closes the underlying file and releases memory
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [in,out] configFilePtr An allocated MetisConfigurationFile that will be NULL'd as output
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void metisConfigurationFile_Release(MetisConfigurationFile **configFilePtr);

#endif /* defined(Metis__metis_ConfigurationFile_h) */
