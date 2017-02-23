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
 * @file longBow_OpenFile.h
 * @ingroup testing
 * @brief LongBow support for files and file descriptors.
 *
 */
#ifndef LongBow_longBow_Files_h
#define LongBow_longBow_Files_h

#include <sys/stat.h>

#include <LongBow/private/longBow_ArrayList.h>

struct longbow_openfile;
/**
 * @typedef LongBowOpenFile
 * @brief A representation of an open file.
 */
typedef struct longbow_openfile LongBowOpenFile;

/**
 * @param [in] fd The file descriptor.
 * @return An allocated LongBowOpenFile instance that must be destroyed via longBowOpenFile_Destroy().
 */
LongBowOpenFile *longBowOpenFile_Create(int fd);

/**
 *
 * @param [in,out] openFilePtr A pointer to a pointer to a valid LongBowOpenFile instance.
 */
void longBowOpenFile_Destroy(LongBowOpenFile **openFilePtr);

/**
 *
 * @param [in] openFile A pointer to a valid LongBowOpenFile instance.
 * @return A nul-terminate C string that must be freed via free(3).
 */
char *longBowOpenFile_ToString(LongBowOpenFile *openFile);

/**
 * Create a list of the currently open files.
 *
 * @return A list of LongBowOpenFile instances for each open file.
 */
LongBowArrayList *longBowOpenFile_CurrentlyOpen(void);

/**
 * Return a nul-terminated C string representing the given `struct stat` pointed to by @p statbuf.
 *
 * @param [in] statbuf A pointer to a valid `struct stat` instance.
 *
 * @return non-NULL A nul-terminated C string that must be deallocated via longBowMemory_Deallocate.
 */
char *longBowOpenFile_StructStatToString(const struct stat *statbuf);
#endif
