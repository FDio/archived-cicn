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

file(GLOB HEADER_FILES "ccnx/*.h")
file(GLOB SOURCE_FILES "ccnx/*.cc")

set(ICNET_HEADER_FILES
        ${CMAKE_BINARY_DIR}/config.hpp
        transport/icnet_rate_estimation.h
        transport/icnet_download_observer.h
        transport/icnet_socket_consumer.h
        transport/icnet_socket.h
        transport/icnet_socket_options_default_values.h
        transport/icnet_socket_options_keys.h
        transport/icnet_common.h
        transport/icnet_socket_producer.h
        transport/icnet_content_store.h
        transport/icnet_transport_vegas.h
        transport/icnet_transport.h
        transport/icnet_transport_raaqm.h
        transport/icnet_transport_vegas_rto_estimator.h
        transport/icnet_transport_raaqm_data_path.h)

set(ICNET_SOURCE_FILES
        transport/icnet_socket_producer.cc
        transport/icnet_socket_consumer.cc
        transport/icnet_transport_vegas.cc
        transport/icnet_transport.cc
        transport/icnet_content_store.cc
        transport/icnet_transport_raaqm.cc
        transport/icnet_transport_vegas_rto_estimator.cc
        transport/icnet_rate_estimation.cc
        transport/icnet_transport_raaqm_data_path.cc)

set(ICNET_CONFIG
        transport/consumer.conf)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

add_library(icnet STATIC ${SOURCE_FILES} ${ICNET_SOURCE_FILES} ${HEADER_FILES} ${ICNET_HEADER_FILES})
add_library(icnet.shared SHARED ${SOURCE_FILES} ${ICNET_SOURCE_FILES})
if(ANDROID_API)
  target_link_libraries(icnet.shared ${LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${CCNX_COMMON_LIBRARIES} ${CCNX_TRANSPORT_RTA_LIBRARIES} ${CCNX_PORTAL_LIBRARIES} ${LIBPARC_LIBRARIES} ${LONGBOW_LIBRARIES})
  target_link_libraries(icnet ${LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES} ${CCNX_COMMON_LIBRARIES} ${CCNX_TRANSPORT_RTA_LIBRARIES} ${CCNX_PORTAL_LIBRARIES} ${LIBPARC_LIBRARIES} ${LONGBOW_LIBRARIES})
else ()
  target_link_libraries(icnet.shared ${LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
  target_link_libraries(icnet ${LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
endif (ANDROID_API)

set_target_properties(icnet.shared PROPERTIES
                      SOVERSION 1
                      VERSION 1.0
                      OUTPUT_NAME icnet)

set(libicnet_libraries
    icnet
    icnet.shared)

foreach(lib ${libicnet_libraries})
    install(TARGETS ${lib} LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
endforeach()

install(FILES ${HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/icnet)
install(FILES ${ICNET_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/icnet)
install(FILES ${ICNET_CONFIG} DESTINATION ${CMAKE_INSTALL_PREFIX}/etc/)
