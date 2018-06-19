# Copyright (c) 2017 Cisco and/or its affiliates.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at:
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Detect the cache size
#
# XXX: TODO: This is a bug when cross compiling. We are detecting the local
# Cache Line size and not the target cache line size.  We should provide some
# way to define this

set(LEVEL1_DCACHE_LINESIZE 32)

if( APPLE ) 
  execute_process(COMMAND sysctl -n hw.cachelinesize
	OUTPUT_VARIABLE LEVEL1_DCACHE_LINESIZE 
    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif( APPLE )

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
  execute_process(COMMAND getconf LEVEL1_DCACHE_LINESIZE
	OUTPUT_VARIABLE LEVEL1_DCACHE_LINESIZE 
    OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()


if ( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "arm"  )
    set(LEVEL1_DCACHE_LINESIZE 64)
endif() 

message("-- Cache line size: ${LEVEL1_DCACHE_LINESIZE}")
