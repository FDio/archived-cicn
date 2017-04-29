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

cmake_minimum_required(VERSION 3.2)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

if ($ENV{CCNX_HOME})
    set(CCNX_HOME $ENV{CCNX_HOME})
else ($ENV{CCNX_HOME})
    set(CCNX_HOME /usr/local)
endif ($ENV{CCNX_HOME})

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -D_GNU_SOURCE")

set(CCNX_LIBRARIES longbow longbow-ansiterm parc ccnx_common ccnx_api_portal ccnx_transport_rta ccnx_api_control ccnx_api_notify)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

set(CCNX_PING_CLIENT_SOURCE_FILES
        iPing_Client.c
        iPing_Common.c
        iPing_Stats.c)

set(CCNX_PING_SERVER_SOURCE_FILES
        iPing_Server.c
        iPing_Common.c)

include_directories(${CCNX_HOME}/include)

link_directories(${CCNX_HOME}/lib)

add_executable(iPing_Client ${CCNX_PING_CLIENT_SOURCE_FILES})
target_link_libraries(iPing_Client ${CCNX_LIBRARIES})
install(TARGETS iPing_Client RUNTIME DESTINATION bin COMPONENT library)

add_executable(iPing_Server ${CCNX_PING_SERVER_SOURCE_FILES})
target_link_libraries(iPing_Server ${CCNX_LIBRARIES})
install(TARGETS iPing_Server RUNTIME DESTINATION bin COMPONENT library)
