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


set(PRODUCER_SOURCE_FILES
        icnet_producer_test.cc)

set(PRODUCER_HELLO_WORLD_SOURCE_FILES
        icnet_producer_hello_world.cc)

set(PRODUCER_PING_SOURCE_FILES
    icnet_ping_server.cc)

set(PRODUCER_PING_SERVER_SERVICE_FILE
    config/iping-server.service)

add_executable(producer-test ${PRODUCER_SOURCE_FILES})
add_executable(producer-hello-world ${PRODUCER_HELLO_WORLD_SOURCE_FILES})
add_executable(iping-server ${PRODUCER_PING_SOURCE_FILES})


target_link_libraries(producer-test icnet ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
target_link_libraries(producer-hello-world icnet ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
target_link_libraries(iping-server icnet ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

install(TARGETS producer-test DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT library)
install(TARGETS producer-hello-world DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT library)
install(TARGETS iping-server DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT library)

if ("${CMAKE_SYSTEM_NAME}" STREQUAL "Linux")
  install(FILES ${PRODUCER_PING_SERVER_SERVICE_FILE} DESTINATION /lib/systemd/system)
endif()
