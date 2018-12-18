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
 * @file parc_StandardOutputStream.h
 * @brief  Standard output stream structures and functions.
 *
 */

#ifndef __PARC_Library__parc_StandardOutputStream__
#define __PARC_Library__parc_StandardOutputStream__

struct PARCStandardOutputStream;
typedef struct PARCStandardOutputStream PARCStandardOutputStream;

#include <parc/algol/parc_OutputStream.h>

PARCOutputStream *parcStandardOutputStream(void);

PARCStandardOutputStream *PARCStandardOutputStream_Create(void);

PARCStandardOutputStream *PARCStandardOutputStream_Acquire(PARCStandardOutputStream *instance);

void PARCStandardOutputStream_Release(PARCStandardOutputStream **instanceP);

#endif /* defined(__PARC_Library__parc_StandardOutputStream__) */
