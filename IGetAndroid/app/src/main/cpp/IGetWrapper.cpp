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
#include <string>
#include <icnet/icnet_http_facade.h>
libl4::http::HTTPClientConnection connection;

extern "C"
JNIEXPORT jbyteArray JNICALL
Java_icn_iget_com_igetandroid_IGetAndroidActivity_downloadFile(JNIEnv *env, jobject instance,
                                                                   jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    std::string name(path);
    env->ReleaseStringUTFChars(path_, path);
    connection.get(name);
    auto response = connection.response();
    if (response.getPayload().size() == 0) {
        jbyte a[] = {};
        jbyteArray ret = env->NewByteArray(0);
        env->SetByteArrayRegion (ret, 0, 0, a);
        return ret;
    }
    jbyteArray ret = env->NewByteArray(response.getPayload().size());
    env->SetByteArrayRegion (ret, 0, response.getPayload().size(), (jbyte *)response.getPayload().data());
    return ret;
}

extern "C"
JNIEXPORT void JNICALL
Java_icn_iget_com_igetandroid_IGetAndroidActivity_stopDownload(JNIEnv *env, jobject instance) {
    connection.stop();
}