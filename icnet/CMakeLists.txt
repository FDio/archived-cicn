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

set(CCNX_HEADER_FILES
        ccnx/icnet_ccnx_common.h
        ccnx/icnet_ccnx_content_object.h
        ccnx/icnet_ccnx_facade.h
        ccnx/icnet_ccnx_interest.h
        ccnx/icnet_ccnx_key_locator.h
        ccnx/icnet_ccnx_key_locator_type.h
        ccnx/icnet_ccnx_local_connector.h
        ccnx/icnet_ccnx_manifest.h
        ccnx/icnet_ccnx_name.h
        ccnx/icnet_ccnx_network_message.h
        ccnx/icnet_ccnx_payload_type.h
        ccnx/icnet_ccnx_pending_interest.h
        ccnx/icnet_ccnx_portal.h
        ccnx/icnet_ccnx_segment.h)

set(CCNX_SOURCE_FILES
        ccnx/icnet_ccnx_segment.cc
        ccnx/icnet_ccnx_portal.cc
        ccnx/icnet_ccnx_pending_interest.cc
        ccnx/icnet_ccnx_network_message.cc
        ccnx/icnet_ccnx_name.cc
        ccnx/icnet_ccnx_manifest.cc
        ccnx/icnet_ccnx_local_connector.cc
        ccnx/icnet_ccnx_key_locator.cc
        ccnx/icnet_ccnx_interest.cc
        ccnx/icnet_ccnx_content_object.cc)

set(TRANSPORT_HEADER_FILES
        ${CMAKE_BINARY_DIR}/config.hpp
        transport/icnet_transport_rate_estimation.h
        transport/icnet_transport_download_observer.h
        transport/icnet_transport_socket_consumer.h
        transport/icnet_transport_socket.h
        transport/icnet_transport_socket_options_default_values.h
        transport/icnet_transport_socket_options_keys.h
        transport/icnet_transport_common.h
        transport/icnet_transport_socket_producer.h
        transport/icnet_transport_content_store.h
        transport/icnet_transport_vegas.h
        transport/icnet_transport_protocol.h
        transport/icnet_transport_raaqm.h
        transport/icnet_transport_vegas_rto_estimator.h
        transport/icnet_transport_raaqm_data_path.h)

set(TRANSPORT_SOURCE_FILES
        transport/icnet_transport_socket_producer.cc
        transport/icnet_transport_socket_consumer.cc
        transport/icnet_transport_vegas.cc
        transport/icnet_transport_protocol.cc
        transport/icnet_transport_content_store.cc
        transport/icnet_transport_raaqm.cc
        transport/icnet_transport_vegas_rto_estimator.cc
        transport/icnet_transport_rate_estimation.cc
        transport/icnet_transport_raaqm_data_path.cc)

set(ERRORS_HEADER_FILES
        errors/icnet_errors_tokenizer_exception.h
        errors/icnet_errors.h
        errors/icnet_errors_malformed_name_exception.h
        errors/icnet_errors_not_implemented_exception.h
        errors/icnet_errors_runtime_exception.h
        errors/icnet_errors_malformed_packet_exception.h)

set(ERRORS_SOURCE_FILES
        errors/icnet_errors_tokenizer_exception.cc
        errors/icnet_errors_malformed_name_exception.cc
        errors/icnet_errors_not_implemented_exception.cc
        errors/icnet_errors_runtime_exception.cc
        errors/icnet_errors_malformed_packet_exception.cc)

set(HTTP_HEADER_FILES
        http/icnet_http_default_values.h
        http/icnet_http_client_connection.h
        http/icnet_http_server_acceptor.h
        http/icnet_http_server_publisher.h
    http/icnet_http_message.h
        http/icnet_http_request.h
    http/icnet_http_response.h
        http/icnet_http_facade.h)

set(HTTP_SOURCE_FILES
        http/icnet_http_client_connection.cc
        http/icnet_http_server_acceptor.cc
        http/icnet_http_server_publisher.cc
        http/icnet_http_request.cc
        http/icnet_http_response.cc)

set(UTILS_HEADER_FILES
        utils/icnet_utils_array.h
        utils/icnet_utils_uri.h
        utils/icnet_utils_daemonizator.h
        utils/icnet_utils_hash.h
        utils/icnet_utils_string_tokenizer.h
        utils/icnet_utils_sharable_vector.h)

set(UTILS_SOURCE_FILES
        utils/icnet_utils_array.cc
        utils/icnet_utils_uri.cc
        utils/icnet_utils_daemonizator.cc
        utils/icnet_utils_hash.cc
        utils/icnet_utils_string_tokenizer.cc)

set(ICNET_CONFIG
        transport/consumer.conf)

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

add_library(icnet STATIC
        ${CCNX_SOURCE_FILES}
        ${TRANSPORT_SOURCE_FILES}
        ${ERRORS_SOURCE_FILES}
        ${HTTP_SOURCE_FILES}
        ${UTILS_SOURCE_FILES})

if (COMPILE_FOR_IOS OR ANDROID_API)
  target_link_libraries(icnet ${LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
  set(libicnet_libraries
        icnet)
else()

  add_library(icnet.shared SHARED
        ${CCNX_SOURCE_FILES}
        ${TRANSPORT_SOURCE_FILES}
        ${ERRORS_SOURCE_FILES}
        ${HTTP_SOURCE_FILES}
        ${UTILS_SOURCE_FILES})

  target_link_libraries(icnet.shared ${LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
  target_link_libraries(icnet ${LIBRARIES} ${CMAKE_THREAD_LIBS_INIT} ${Boost_LIBRARIES})
  set_target_properties(icnet.shared PROPERTIES
        SOVERSION 1
        VERSION 1.0
        OUTPUT_NAME icnet)

  set(libicnet_libraries
        icnet
        icnet.shared)
endif()

foreach(lib ${libicnet_libraries})
    install(TARGETS ${lib} COMPONENT library LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
endforeach()

install(FILES ${CCNX_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/icnet COMPONENT headers)
install(FILES ${TRANSPORT_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/icnet COMPONENT headers)
install(FILES ${HTTP_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/icnet COMPONENT headers)
install(FILES ${ERRORS_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/icnet COMPONENT headers)
install(FILES ${UTILS_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_PREFIX}/include/icnet COMPONENT headers)
install(FILES ${ICNET_CONFIG} DESTINATION ${CMAKE_INSTALL_PREFIX}/etc/ COMPONENT library)
