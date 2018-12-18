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

#include <stdio.h>
#include <stdlib.h>

#include "parc_SimpleBufferPool.h"

int
main(int argc, char *argv[argc])
{
    PARCSimpleBufferPool *pool = parcSimpleBufferPool_Create(3, 10);

    PARCBuffer *buffer = parcSimpleBufferPool_GetInstance(pool);
    parcBuffer_Release(&buffer);

    buffer = parcSimpleBufferPool_GetInstance(pool);
    parcBuffer_Release(&buffer);

    parcSimpleBufferPool_Release(&pool);

    return 0;
}
