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
 * @file longBow_ClipBoard.h
 * @brief <#Brief Description#>
 *
 * <#Detailed Description#>
 *
 */
#ifndef longBow_ClipBoard_h
#define longBow_ClipBoard_h

#include <stdbool.h>
#include <stdint.h>

struct LongBowClipBoard;
typedef struct LongBowClipBoard LongBowClipBoard;

void longBowClipBoard_Destroy(LongBowClipBoard **pointer);

LongBowClipBoard *longBowClipBoard_Create(void);

void *longBowClipBoard_Get(const LongBowClipBoard *clipBoard, const char *name);

char *longBowClipBoard_GetAsCString(const LongBowClipBoard *clipBoard, const char *name);

uint64_t longBowClipBoard_GetAsInt(const LongBowClipBoard *clipBoard, const char *name);

void *longBowClipBoard_Set(LongBowClipBoard *clipBoard, const char *name, void *value);

void *longBowClipBoard_SetInt(LongBowClipBoard *clipBoard, const char *name, uint64_t value);

void *longBowClipBoard_SetCString(LongBowClipBoard *clipBoard, const char *name, char *value);

bool longBowClipBoard_Exists(const LongBowClipBoard *clipBoard, const char *name);

bool longBowClipBoard_Delete(LongBowClipBoard *clipBoard, const char *name);

#endif /* longBow_ClipBoard_h */
