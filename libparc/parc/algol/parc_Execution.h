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
 * @file parc_Execution.h
 * @ingroup datastructures
 * @brief PARC Execution Status
 * An extensible set of status values used to communicate
 * out-of-band or exceptional conditions as return values.
 *
 */
#ifndef parc_Execution_h
#define parc_Execution_h

#include <stdbool.h>

struct PARCExecution;
typedef struct PARCExecution PARCExecution;

extern PARCExecution *PARCExecution_OK;
extern PARCExecution *PARCExecution_Interrupted;
extern PARCExecution *PARCExecution_IOError;
extern PARCExecution *PARCExecution_Timeout;

char *parcExecution_GetMessage(const PARCExecution *exec);

bool parcExecution_Is(const PARCExecution *exec, const PARCExecution *other);

#endif /* parc_Status_h */
