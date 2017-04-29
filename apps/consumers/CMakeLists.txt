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

set(CONSUMER_SOURCE_FILES icnet_consumer_test.cc)
set(IGET_SOURCE_FILES icnet_iget.cc)
set(CONSUMERDASH_SOURCE_FILES icnet_consumer_dash.cc)

add_executable(consumer-test ${CONSUMER_SOURCE_FILES})
add_executable(iget ${IGET_SOURCE_FILES})
add_executable(consumer-dash ${CONSUMERDASH_SOURCE_FILES})

target_link_libraries(consumer-test icnet ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
target_link_libraries(iget icnet ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
target_link_libraries(consumer-dash icnet ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})

install(TARGETS consumer-test DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT library)
install(TARGETS iget DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT library)
install(TARGETS consumer-dash DESTINATION ${CMAKE_INSTALL_PREFIX}/bin COMPONENT library)
