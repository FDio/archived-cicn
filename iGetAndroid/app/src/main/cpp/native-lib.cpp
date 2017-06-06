
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
#include <string>

#include <icnet/icnet_http_facade.h>



extern "C"
JNIEXPORT jstring JNICALL
Java_com_iget_ccnx_igetandroid_iGetActivity_downloadFile(JNIEnv *env, jobject instance,
                                                            jstring path_) {
    const char *path = env->GetStringUTFChars(path_, 0);
    std::string name(path);
    libl4::http::HTTPClientConnection connection;
    connection.get(name);
    std::string str(reinterpret_cast<const char *>(connection.response().data()),
                    connection.response().size());
    return env->NewStringUTF(str.c_str());
}