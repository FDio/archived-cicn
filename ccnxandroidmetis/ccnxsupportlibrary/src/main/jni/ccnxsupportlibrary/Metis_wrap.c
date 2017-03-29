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

#include <string.h>
#include <stdio.h>
#include <getopt.h>
#include <sys/param.h>
#include <sys/utsname.h>
#include <pthread.h>
#include <dirent.h>
#include <android/log.h>
#include "Metis_wrap.h"
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>
#include <ccnx/forwarder/metis/core/metis_System.h>
#include <ccnx/forwarder/metis/content_store/metis_ContentStoreInterface.h>
#include <ccnx/api/control/cpi_Listener.h>
#include <ccnx/api/control/cpi_InterfaceSet.h>


static bool _isRunning = false;


static MetisForwarder *metis;

static void
_setLogLevelToLevel(int logLevelArray[MetisLoggerFacility_END], MetisLoggerFacility facility, const char *levelString)
{
    PARCLogLevel level = parcLogLevel_FromString(levelString);

    if (level < PARCLogLevel_All) {
        // we have a good facility and level
        logLevelArray[facility] = level;
    } else {
        printf("Invalid log level string %s\n", levelString);
        __android_log_print(ANDROID_LOG_DEBUG, "Metis Wrap", "Invalid log level stringa %s", levelString);
    }
}


static void
_setLogLevel(int logLevelArray[MetisLoggerFacility_END], const char *string)
{
    char *tofree = parcMemory_StringDuplicate(string, strlen(string));
    char *p = tofree;

    char *facilityString = strsep(&p, "=");
    if (facilityString) {
        char *levelString = p;

        if (strcasecmp(facilityString, "all") == 0) {
            for (MetisLoggerFacility facility = 0; facility < MetisLoggerFacility_END; facility++) {
                _setLogLevelToLevel(logLevelArray, facility, levelString);
            }
        } else {
            MetisLoggerFacility facility;
            for (facility = 0; facility < MetisLoggerFacility_END; facility++) {
                if (strcasecmp(facilityString, metisLogger_FacilityString(facility)) == 0) {
                    break;
                }
            }

            if (facility < MetisLoggerFacility_END) {
                _setLogLevelToLevel(logLevelArray, facility, levelString);
            } else {
                __android_log_print(ANDROID_LOG_DEBUG, "Metis Wrap", "Invalid facility string %s", facilityString);
            }
        }
    }
    parcMemory_Deallocate((void **) &tofree);
}



JNIEXPORT void JNICALL Java_com_metis_ccnx_ccnxsupportlibrary_Metis_start
  (JNIEnv *env, jobject obj, jstring path)
{
    if (!_isRunning) {
        metis = metisForwarder_Create(NULL);
        MetisConfiguration *configuration = metisForwarder_GetConfiguration(metis);
        metisConfiguration_SetObjectStoreSize(configuration, 0);
        metisConfiguration_StartCLI(configuration, 2001);
        if (path != NULL) {
        __android_log_print(ANDROID_LOG_DEBUG, "Metis Wrap","qui");
            const char *configFileName = (*env)->GetStringUTFChars(env, path, 0);
            __android_log_print(ANDROID_LOG_DEBUG, "Metis Wrap", "configuration file %s", configFileName);
            metisForwarder_SetupFromConfigFile(metis, configFileName);
            __android_log_print(ANDROID_LOG_DEBUG, "Metis Wrap","config from file");
        } else {
         __android_log_print(ANDROID_LOG_DEBUG, "Metis Wrap","qua");
            metisForwarder_SetupAllListeners(metis, PORT_NUMBER, NULL);
        }
        MetisDispatcher *dispatcher = metisForwarder_GetDispatcher(metis);
        __android_log_print(ANDROID_LOG_DEBUG, "Metis Wrap","dispatcher");
        _isRunning = true;
        __android_log_print(ANDROID_LOG_DEBUG, "Metis Wrap","true");
        metisDispatcher_Run(dispatcher);
        __android_log_print(ANDROID_LOG_DEBUG, "Metis Wrap","run");
    }

 }

JNIEXPORT void JNICALL Java_com_metis_ccnx_ccnxsupportlibrary_Metis_stop
  (JNIEnv *env, jobject obj)
{
    if(_isRunning) {
        __android_log_print(ANDROID_LOG_DEBUG, "Metis Wrap", "%s", "stopping metis...");
        metisForwarder_Destroy(&metis);
        _isRunning = false;
   }
}


JNIEXPORT jboolean JNICALL Java_com_metis_ccnx_ccnxsupportlibrary_Metis_isRunning
  (JNIEnv *env, jobject obj) {
        __android_log_print(ANDROID_LOG_DEBUG, "Metis Wrap", "%s %d", " metis is running", _isRunning);
        return _isRunning;
}


