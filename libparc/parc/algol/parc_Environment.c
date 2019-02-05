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


#ifndef _WIN32
#include <unistd.h>
#include <pwd.h>
#endif

#include <config.h>
#include <sys/types.h>
#include <stdlib.h>
#include <parc/algol/parc_Environment.h>
#include <parc/algol/parc_File.h>

const char *
parcEnvironment_GetHomeDirectory(void)
{
    char *result = getenv("HOME");
    return result;
}

PARCFile *
parcEnvironment_HomeDirectory(void)
{

#ifdef _WIN32
    char *pValue;
    size_t len;
    errno_t err = _dupenv_s(&pValue, &len, "USERPROFILE");
    if (err != 0) {
        return parcFile_Create(pValue);
    }
    else {
        return NULL;
    }
#else
    char *path;
    if ((path = getenv("HOME")) == NULL) {
        path = getpwuid(getuid())->pw_dir;
    }
    return parcFile_Create(path);
#endif

}
