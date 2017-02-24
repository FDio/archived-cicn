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
 * @file longBow_Properties.h
 * @brief A simple properties store.
 *
 */
#ifndef __LongBow__longBow_Properties__
#define __LongBow__longBow_Properties__

#include <stdbool.h>
#include <stdlib.h>

struct LongBowProperties;
typedef struct LongBowProperties LongBowProperties;

LongBowProperties *longBowProperties_Create(void);

const char *longBowProperties_Get(const LongBowProperties *properties, const char *name);

size_t longBowProperties_Length(const LongBowProperties *properties);

bool longBowProperties_Set(LongBowProperties *properties, const char *name, const char *value);

bool longBowProperties_Exists(const LongBowProperties *properties, const char *name);

bool longBowProperties_Delete(LongBowProperties *properties, const char *name);

void longBowProperties_Destroy(LongBowProperties **propertiesPtr);

char *longBowProperties_ToString(const LongBowProperties *properties);

#endif /* defined(__LongBow__longBow_Properties__) */
