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

#include <jni.h>


#ifndef _Included_com_cisco_ccnx_service_CCNxService
#define _Included_com_cisco_ccnx_service_CCNxService
#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_com_cisco_ccnx_service_CCNxService_startNfd(JNIEnv*, jclass, jobject);

JNIEXPORT void JNICALL
Java_com_cisco_ccnx_service_CCNxService_stopNfd(JNIEnv*, jclass);

JNIEXPORT void JNICALL
Java_com_cisco_ccnx_service_CCNxService_startIGet(JNIEnv *env, jobject obj, jstring downloadPath, jstring destinationPath);

JNIEXPORT jobject JNICALL
Java_com_cisco_ccnx_service_CCNxService_getNfdLogModules(JNIEnv*, jclass);

#ifdef __cplusplus
}
#endif
#endif
