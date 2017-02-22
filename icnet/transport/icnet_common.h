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

#ifndef ICNET_COMMON_H_
#define ICNET_COMMON_H_

// require C++11
#if __cplusplus < 201103L && !defined(__GXX_EXPERIMENTAL_CXX0X__)
#error "icnet needs to be compiled using the C++11 standard"
#endif

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <functional>
#include <limits>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <unistd.h>
#include <utility>
#include <fstream>
#include <chrono>

#include "config.hpp"
#include "icnet_ccnx_facade.h"

#if defined(__GNUC__) || defined(__clang__)
#  define DEPRECATED(func) func __attribute__ ((deprecated))
#elif defined(_MSC_VER)
#  define DEPRECATED(func) __declspec(deprecated) func
#else
#  pragma message("DEPRECATED not implemented")
#  define DEPRECATED(func) func
#endif

#endif // ICNET_COMMON_H_
