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
 * This is from the version 0 codec.  All the test vectors in this directory (e.g. interest_nameA.h)
 * are encoded using these constants.  These are no longer used for any functional code, only to interpret the test vectors.
 *
 */

#ifndef Libccnx_tlv_Schema_h
#define Libccnx_tlv_Schema_h

#define T_INVALID 0xFFFF

// not an actual type, but a virtual group
#define T_VIRTUAL 0xFFFE
#endif // Libccnx_tlv_Schema_h
