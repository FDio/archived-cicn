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


#include <config.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include <LongBow/longBow_Runtime.h>
#include <parc/algol/parc_Memory.h>
#include <parc/algol/parc_Object.h>
#include <parc/algol/parc_ArrayList.h>
#include <parc/algol/parc_List.h>
#include <parc/assert/parc_Assert.h>
#include <ccnx/forwarder/metis/config/metis_ConfigurationFile.h>
#include <ccnx/forwarder/metis/config/metis_Configuration.h>
#include <ccnx/forwarder/metis/config/metis_ControlState.h>
#include <ccnx/forwarder/metis/config/metisControl_Root.h>

struct metis_configuration_file {
    MetisForwarder *metis;
    const char *filename;
    FILE *fh;

    size_t linesRead;

    // our custom state machine.
    MetisControlState *controlState;
};


/**
 * Called by the command parser for each command.
 *
 * The command parser will make a CCNxControl message inside the CCNxMetaMessage and send it here.
 * This function must return a ACK or NACK in a CCNxControl in a CCNxMetaMessage.
 *
 * @param [in] userdata A void * to MetisConfigurationFile
 * @param [in] msg The CCNxControl message to process
 *
 * @retval CCNxMetaMessage A CPI ACK or NACK in a CCNxControl in a CCNxMetaMessage
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static CCNxMetaMessage *
_writeRead(void *userdata, CCNxMetaMessage *msgin)
{
    MetisConfigurationFile *configFile = (MetisConfigurationFile *) userdata;

    CCNxControl *request = ccnxMetaMessage_GetControl(msgin);
    CCNxControl *response = metisConfiguration_ReceiveControl(metisForwarder_GetConfiguration(configFile->metis), request, 0);

    CCNxMetaMessage *msgout = ccnxMetaMessage_CreateFromControl(response);
    ccnxControl_Release(&response);

    return msgout;
}

/**
 * Removes leading whitespace (space + tab).
 *
 * If the string is all whitespace, the return value will point to the terminating '\0'.
 *
 * @param [in] str A null-terminated c-string
 *
 * @retval non-null A pointer in to string of the first non-whitespace
 *
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static char *
_stripLeadingWhitespace(char *str)
{
    while (isspace(*str)) {
        str++;
    }
    return str;
}

/**
 * Removes trailing whitespace
 *
 * Inserts a NULL after the last non-whitespace character, modiyfing the input string.
 *
 * @param [in] str A null-terminated c-string
 *
 * @return non-null A pointer to the input string
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static char *
_stripTrailingWhitespace(char *str)
{
    char *p = str + strlen(str) - 1;
    while (p > str && isspace(*p)) {
        p--;
    }

    // cap it.  If no whitespace, p+1 == str + strlen(str), so will overwrite the
    // current null.  If all whitespace p+1 == str+1.  For an empty string, p+1 = str.
    *(p + 1) = 0;

    // this does not catch the case where the entire string is whitespace
    if (p == str && isspace(*p)) {
        *p = 0;
    }

    return str;
}

/**
 * Removed leading and trailing whitespace
 *
 * Modifies the input string (may add a NULL at the end).  Will return
 * a pointer to the first non-whitespace character or the terminating NULL.
 *
 * @param [in] str A null-terminated c-string
 *
 * @return non-null A pointer in to the input string
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
static char *
_trim(char *str)
{
    return _stripTrailingWhitespace(_stripLeadingWhitespace(str));
}

/**
 * Parse a string in to a PARCList with one word per element
 *
 * The string passed will be modified by inserting NULLs after each token.
 *
 * @param [in] str A c-string (will be modified)
 *
 * @retval non-null A PARCList where each item is a single word
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
static PARCList *
_parseArgs(char *str)
{
    PARCList *list = parcList(parcArrayList_Create(NULL), PARCArrayListAsPARCList);

    const char delimiters[] = " \t";

    char *token;
    while ((token = strsep(&str, delimiters)) != NULL) {
        parcList_Add(list, token);
    }

    return list;
}

// =============================================================

static void
_destroy(MetisConfigurationFile **configFilePtr)
{
    MetisConfigurationFile *configFile = *configFilePtr;
    parcMemory_Deallocate((void **) &configFile->filename);

    if (configFile->fh != NULL) {
        fclose(configFile->fh);
    }

    metisControlState_Destroy(&configFile->controlState);
}

parcObject_ExtendPARCObject(MetisConfigurationFile, _destroy, NULL, NULL, NULL, NULL, NULL, NULL);

parcObject_ImplementRelease(metisConfigurationFile, MetisConfigurationFile);

MetisConfigurationFile *
metisConfigurationFile_Create(MetisForwarder *metis, const char *filename)
{
    parcAssertNotNull(metis, "Parameter metis must be non-null");
    parcAssertNotNull(filename, "Parameter filename must be non-null");

    MetisConfigurationFile *configFile = parcObject_CreateInstance(MetisConfigurationFile);

    if (configFile) {
        configFile->linesRead = 0;
        configFile->metis = metis;
        configFile->filename = parcMemory_StringDuplicate(filename, strlen(filename));
        parcAssertNotNull(configFile->filename, "Could not copy string '%s'", filename);

        // setup the control state for the command parser
        configFile->controlState = metisControlState_Create(configFile, _writeRead);

        // we do not register Help commands
        metisControlState_RegisterCommand(configFile->controlState, metisControlRoot_Create(configFile->controlState));

        // open the file and make sure we can read it
        configFile->fh = fopen(configFile->filename, "r");

        if (configFile->fh) {
            if (metisLogger_IsLoggable(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug)) {
                metisLogger_Log(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Debug, __func__,
                                "Open config file %s",
                                configFile->filename);
            }
        } else {
            if (metisLogger_IsLoggable(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Error)) {
                metisLogger_Log(metisForwarder_GetLogger(metis), MetisLoggerFacility_Config, PARCLogLevel_Error, __func__,
                                "Could not open config file %s: (%d) %s",
                                configFile->filename,
                                errno,
                                strerror(errno));
            }

            // failure cleanup the object -- this nulls it so final return null be NULL
            metisConfigurationFile_Release(&configFile);
        }
    }
    return configFile;
}


bool
metisConfigurationFile_Process(MetisConfigurationFile *configFile)
{
    parcAssertNotNull(configFile, "Parameter configFile must be non-null");

    // default to a "true" return value and only set to false if we encounter an error.
    bool success = true;

    #define BUFFERLEN 2048
    char buffer[BUFFERLEN];

    configFile->linesRead = 0;

    // always clear errors and fseek to start of file in case we get called multiple times.
    clearerr(configFile->fh);
    rewind(configFile->fh);

    while (success && fgets(buffer, BUFFERLEN, configFile->fh) != NULL) {
        configFile->linesRead++;

        char *stripedBuffer = _trim(buffer);
        if (strlen(stripedBuffer) > 0) {
            if (stripedBuffer[0] != '#') {
                // not empty and not a comment

                // _parseArgs will modify the string
                char *copy = parcMemory_StringDuplicate(stripedBuffer, strlen(stripedBuffer));
                PARCList *args = _parseArgs(copy);
                MetisCommandReturn result = metisControlState_DispatchCommand(configFile->controlState, args);

                // we ignore EXIT from the configuration file
                if (result == MetisCommandReturn_Failure) {
                    if (metisLogger_IsLoggable(metisForwarder_GetLogger(configFile->metis), MetisLoggerFacility_Config, PARCLogLevel_Error)) {
                        metisLogger_Log(metisForwarder_GetLogger(configFile->metis), MetisLoggerFacility_Config, PARCLogLevel_Error, __func__,
                                        "Error on input file %s line %d: %s",
                                        configFile->filename,
                                        configFile->linesRead,
                                        stripedBuffer);
                    }
                    success = false;
                }
                parcList_Release(&args);
                parcMemory_Deallocate((void **) &copy);
            }
        }
    }

    if (ferror(configFile->fh)) {
        if (metisLogger_IsLoggable(metisForwarder_GetLogger(configFile->metis), MetisLoggerFacility_Config, PARCLogLevel_Error)) {
            metisLogger_Log(metisForwarder_GetLogger(configFile->metis), MetisLoggerFacility_Config, PARCLogLevel_Error, __func__,
                            "Error on input file %s line %d: (%d) %s",
                            configFile->filename,
                            configFile->linesRead,
                            errno,
                            strerror(errno));
        }
        success = false;
    }

    return success;
}

