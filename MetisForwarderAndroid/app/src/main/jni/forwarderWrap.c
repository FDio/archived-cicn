/*
 * Copyright (c) 2018 Cisco and/or its affiliates.
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

#include <jni.h>
#include <unistd.h>
#include <stdio.h>
#include <android/log.h>
#include <stdbool.h>
#include <ccnx/forwarder/metis/core/metis_Forwarder.h>

static bool _isRunning = false;
MetisForwarder *metis = NULL;


JNIEXPORT jboolean JNICALL
Java_icn_forwarder_com_supportlibrary_Forwarder_isRunning(JNIEnv *env, jobject instance) {
    return _isRunning;
}

JNIEXPORT void JNICALL
Java_icn_forwarder_com_supportlibrary_Forwarder_start(JNIEnv *env, jobject instance,
                                                      jstring path_) {
    if (!_isRunning) {
        __android_log_print(ANDROID_LOG_DEBUG, "HicnFwdWrap", "starting HicnFwd...");
        metis = metisForwarder_Create(NULL);
        MetisConfiguration *configuration = metisForwarder_GetConfiguration(metis);
        metisConfiguration_SetObjectStoreSize(configuration, 0);
        metisConfiguration_StartCLI(configuration, 2001);
        if (path_) {
            const char *configFileName = (*env)->GetStringUTFChars(env, path_, 0);
            metisForwarder_SetupFromConfigFile(metis, configFileName);
        } else {
            metisForwarder_SetupAllListeners(metis, PORT_NUMBER, NULL);
        }
        MetisDispatcher *dispatcher = metisForwarder_GetDispatcher(metis);
        _isRunning = true;
        metisDispatcher_Run(dispatcher);
    }
}

JNIEXPORT void JNICALL
Java_icn_forwarder_com_supportlibrary_Forwarder_stop(JNIEnv *env, jobject instance) {
    if (_isRunning) {
        __android_log_print(ANDROID_LOG_DEBUG, "MetisForwarderWrap", "stopping Metis...");
        metisDispatcher_Stop(metisForwarder_GetDispatcher(metis));
        sleep(2);
        metisForwarder_Destroy(&metis);
        _isRunning = false;
    }
}
