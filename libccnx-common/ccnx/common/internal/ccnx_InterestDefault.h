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
 * @file ccnx_InterestDefault.h
 * @brief Default values for various fields in an Interest
 *
 * Declares several constants that may be used when creating an Interest with default values.
 * These may be used in a V0 or V1 Interest.
 *
 */

#ifndef __CCNx_Common__ccnx_InterestDefault__
#define __CCNx_Common__ccnx_InterestDefault__

#include <inttypes.h>
#include <ccnx/common/internal/ccnx_InterestPayloadIdMethod.h>

/**
 * May be used in calls to create an Interest with the default lifetime.  An Interest with
 * the default lifetime does not encode the field in the wire format.
 */
extern const uint32_t CCNxInterestDefault_LifetimeMilliseconds;

/**
 * May be used in calls to create an Interest with the default hoplimit.
 */
extern const uint32_t CCNxInterestDefault_HopLimit;

/**
 * May be used in calls to create an Interest with the default payload id method.
 *
 * The PayloadIdMethod is a field inside the Interest that describes how the InterestPayloadId in the Name
 * was calcuated.
 */
extern const CCNxInterestPayloadIdMethod CCNxInterestDefault_PayloadIdMethod;

#endif /* defined(__CCNx_Common__ccnx_InterestDefault__) */
