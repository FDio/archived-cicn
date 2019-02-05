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
 * @file parc_Unsigned.h
 * @ingroup types
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef PARCLibrary_parc_Unsigned
#define PARCLibrary_parc_Unsigned

#include <parc/algol/parc_JSON.h>
#include <parc/algol/parc_HashCode.h>

typedef struct PARCUnsigned PARCUnsigned;
extern parcObjectDescriptor_Declaration(PARCUnsigned);

/**
 */
PARCUnsigned *parcUnsigned_Acquire(const PARCUnsigned *instance);

#ifdef PARCLibrary_DISABLE_VALIDATION
#  define parcUnsigned_OptionalAssertValid(_instance_)
#else
#  define parcUnsigned_OptionalAssertValid(_instance_) parcUnsigned_AssertValid(_instance_)
#endif

/**
 */
void parcUnsinged_AssertValid(const PARCUnsigned *instance);

/**
 */
PARCUnsigned *parcUnsigned_Create(unsigned x);


/**
 */
int parcUnsigned_Compare(const PARCUnsigned *instance, const PARCUnsigned *other);

/**
 */
PARCUnsigned *parcUnsigned_Copy(const PARCUnsigned *original);

/**
 */
void parcUnsigned_Display(const PARCUnsigned *instance, int indentation);

/**
 */
bool parcUnsigned_Equals(const PARCUnsigned *x, const PARCUnsigned *y);

/**
 */
PARCHashCode parcUnsigned_HashCode(const PARCUnsigned *instance);

/**
 */
bool parcUnsigned_IsValid(const PARCUnsigned *instance);

/**
 */
void parcUnsigned_Release(PARCUnsigned **instancePtr);

/**
 */
PARCJSON *parcUnsigned_ToJSON(const PARCUnsigned *instance);

/**
 */
char *parcUnsigned_ToString(const PARCUnsigned *instance);

/**
 */
unsigned parcUnsigned_GetUnsigned(const PARCUnsigned *istance);
#endif
