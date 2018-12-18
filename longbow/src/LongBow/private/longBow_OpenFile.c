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
 */
#include <config.h>

#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <poll.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/param.h>
#include <sys/resource.h>
#include <string.h>

#include <LongBow/private/longBow_OpenFile.h>
#include <LongBow/private/longBow_Memory.h>
#include <LongBow/private/longBow_String.h>

struct longbow_openfile {
    char *fullPathName;
    struct stat statbuf;
    int fd;
};

#if defined(_WIN64)
static char *
_getFullPathNameFromFd(int fd)
{
    return strdup("./");
}
#elif defined(_WIN32)
static char *
_getFullPathNameFromFd(int fd)
{
    return strdup("./");
}
#elif defined(__APPLE__)
static char *
_getFullPathNameFromFd(int fd)
{
    char filePath[PATH_MAX];

    if (fcntl(fd, F_GETPATH, filePath) != -1) {
        return strdup(filePath);
    }
    return NULL;
}
#elif defined(__linux)
static char *
_getFullPathNameFromFd(int fd)
{
    return strdup("./");
}
#elif defined(__unix) // all unices not caught above
static char *
_getFullPathNameFromFd(int fd)
{
    return strdup("./");
}
#elif defined(__posix)
static char *
_getFullPathNameFromFd(int fd)
{
    return strdup("./");
}
#endif

LongBowOpenFile *
longBowOpenFile_Create(int fd)
{
    LongBowOpenFile *result = longBowMemory_Allocate(sizeof(LongBowOpenFile));
    if (result != NULL) {
        result->fd = fd;

        if (fstat(fd, &result->statbuf) == 0) {
            result->fullPathName = _getFullPathNameFromFd(fd);
        }
    }
    return result;
}

void
longBowOpenFile_Destroy(LongBowOpenFile **openFilePtr)
{
    longBowMemory_Deallocate((void **) openFilePtr);
}

char *
longBowOpenFile_StructStatToString(const struct stat *statbuf)
{
    LongBowString *string = longBowString_CreateFormat("0x%" PRIx16 " %3ld %10d %10d %6lld",
                                                       statbuf->st_mode,
                                                       (long) statbuf->st_nlink,
                                                       statbuf->st_uid,
                                                       statbuf->st_gid,
                                                       (long long) statbuf->st_size);
    char *result = longBowString_ToString(string);

    return result;
}

char *
longBowOpenFile_ToString(LongBowOpenFile *openFile)
{
    char *statString = longBowOpenFile_StructStatToString(&(openFile->statbuf));
    LongBowString *string = longBowString_CreateFormat("%d %s %s", openFile->fd, statString, openFile->fullPathName);
    free(statString);
    char *result = longBowString_ToString(string);

    return result;
}

LongBowArrayList *
longBowOpenFile_CurrentlyOpen(void)
{
    struct rlimit rlimit;

    bool success = getrlimit(RLIMIT_NOFILE, &rlimit) == 0;
    if (!success) {
        return NULL;
    }
    // Here is a potential problem:  struct rlimit specifies rlim_cur as a 64 bit value (rlim_t), but poll only takes an
    // unsigned int's worth of file descriptors.

    LongBowArrayList *list = longBowArrayList_Create(longBowArrayList_StdlibFreeFunction);

    for (nfds_t i = 0; i < (nfds_t) rlimit.rlim_cur; i++) {
        int flags = fcntl((int) i, F_GETFD);
        if (flags != -1) {
            LongBowOpenFile *openFile = longBowOpenFile_Create((int) i);
            longBowArrayList_Add(list, openFile);
        }
    }

    return list;
}
