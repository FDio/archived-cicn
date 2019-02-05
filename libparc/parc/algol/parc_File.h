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
 * @file parc_File.h
 * @ingroup inputoutput
 * @brief File manipulation
 *
 *
 */
#ifndef libparc_parc_File_h
#define libparc_parc_File_h

#include <stdbool.h>

#include <parc/algol/parc_BufferComposer.h>

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcFile_OptionalAssertValid(_instance_)
#else
#  define parcFile_OptionalAssertValid(_instance_) parcFile_AssertValid(_instance_)
#endif

struct parc_file;
typedef struct parc_file PARCFile;

/**
 * Creates a `PARCFile` object named by pathname.
 *
 * This operation does not imply any I/O operations.
 * The PARCFile instance only represents the pathname,
 * and does not necessarily reference a real file.
 *
 * @param [in] pathname is a pointer to a char array (string)
 * @return A pointer to an instance of `PARCFile`
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
PARCFile *parcFile_Create(const char *pathname);

/**
 * Acquire a new reference to an instance of `PARCFile`.
 *
 * The reference count to the instance is incremented.
 *
 * @param [in] file The instance of `PARCFile` to which to refer.
 *
 * @return The same value as the input parameter @p file
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
PARCFile *parcFile_Acquire(const PARCFile *file);

/**
 * Assert that an instance of `PARCFile` is valid.
 *
 * Valid means the internal state of the type is consistent with its required current or future behaviour.
 * This may include the validation of internal instances of types.
 *
 * @param [in] instance A pointer to a `PARCFile` instance.
 *
 * Example:
 * @code
 * {
 *     PARCFile *file = parcFile_Create("/tmp/foo");
 *
 *     parcFile_AssertValid(file);
 * }
 * @endcode
 */
void parcFile_AssertValid(const PARCFile *instance);

/**
 * Release a `PARCFile` reference.
 *
 * Only the last invocation where the reference count is decremented to zero,
 * will actually destroy the `PARCFile`.
 *
 * @param [in,out] filePtr is a pointer to the `PARCFile` reference.
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
void parcFile_Release(PARCFile **filePtr);

/**
 * Create a new file on storage.
 *
 * @param [in] file A pointer to an instance of `PARCFile`
 *
 * @return true if succesful, false if not
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcFile_CreateNewFile(const PARCFile *file);

/**
 * Return true if the PARCFile exists on storage.
 *
 * If the pathname can be stat(2)'d, then it exists.
 *
 * @param [in] file A pointer to a `PARCFile` instance.
 * @return true if the file exists, false otherwise
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcFile_Exists(const PARCFile *file);

/**
 * Create a new directory.
 *
 * @param [in] file A pointer to a `PARCFile` instance.
 * @return true if the pathname exists, false otherwise
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcFile_Mkdir(const PARCFile *file);

/**
 * True if the specified `PARCFile` is an existing directory on storage.
 *
 * @param [in] file A pointer to a `PARCFile` instance.
 *
 * @return true if specified `PARCFile` is an existing directory on storage, false if not
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
bool parcFile_IsDirectory(const PARCFile *file);

/**
 * Deletes the file or directory on storage.
 *
 * For a directory, it does a recursive delete.
 *
 * @param [in] file The instance of `PARCFile` to be deleted.
 * @return `true` if everything deleted, false otherwise
 *
 * Example:
 * @code
 * <#example#>
 * @endcode
 */
bool parcFile_Delete(const PARCFile *file);

/**
 * Append a representation of the specified `PARCFile` instance to the given {@link PARCBufferComposer}.
 *
 * @param [in] file A pointer to the `PARCFile` instance whose contents should be appended to to string.
 * @param [in,out] string A pointer to the `PARCBufferComposer` instance to which the contents of file will be appended.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL The instance of `PARCBufferComposer` with the appended contents.
 *
 * Example:
 * @code
 * {
 *     PARCBufferComposer *result = parcBufferComposer_Create();
 *     PARCFile *instance = parcFile_Create("/tmp/foo");
 *
 *     parcFile_BuildString(instance, result);
 *
 *     PARCBuffer *string = parcBufferComposer_FinalizeBuffer(result);
 *     printf("File: %s\n", parcBuffer_ToString(string));
 *     parcBuffer_Release(&string);
 *
 *     parcBufferComposer_Release(&result);
 * }
 * @endcode
 */
PARCBufferComposer *parcFile_BuildString(const PARCFile *file, PARCBufferComposer *string);

/**
 * <#One Line Description#>
 *
 * <#Paragraphs Of Explanation#>
 *
 * @param [<#in#> | <#out#> | <#in,out#>] <#name#> <#description#>
 *
 * @return <#value#> <#explanation#>
 *
 * Example:
 * @code
 * {
 *     <#example#>
 * }
 * @endcode
 */
size_t parcFile_GetFileSize(const PARCFile *file);

/**
 * Create a PARCFile representation of the home directory of the current user.
 *
 * The return value must be released via `parcFile_Release`.
 *
 * @return non-NULL A pointer to a valid PARCFile instance.
 * @return NULL An error occurred.
 *
 * Example:
 * @code
 * {
 *     PARCFile *directory = parcFile_CreateHome();
 * }
 * @endcode
 */
PARCFile *parcFile_CreateHome(void);

/**
 * Produce a null-terminated string representation of the specified `PARCFile` instance.
 *
 * The non-null result must be freed by the caller via {@link parcMemory_Deallocate}.
 *
 * @param [in] file A pointer to the `PARCFile` instance.
 *
 * @return NULL Cannot allocate memory.
 * @return non-NULL A pointer to an allocated, null-terminated C string that must be deallocated via {@link parcMemory_Deallocate}.
 *
 * Example:
 * @code
 * {
 *     PARCFile *instance = parcFile_Create("/tmp/foo");
 *
 *     char *string = parcFile_ToString(instance);
 *
 *     if (string != NULL) {
 *         printf("Hello: %s\n", string);
 *         parcMemory_Deallocate((void **)&string);
 *     } else {
 *         printf("Cannot allocate memory\n");
 *     }
 *
 *     parcFile_Release(&instance);
 * }
 * @endcode
 *
 * @see {@link parcFile_BuildString}
 */
char *parcFile_ToString(const PARCFile *file);
#endif // libparc_parc_File_h
