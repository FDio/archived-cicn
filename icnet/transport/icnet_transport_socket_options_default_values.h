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

#ifndef ICNET_SOCKET_OPTIONS_DEFAULT_VALUES_H_
#define ICNET_SOCKET_OPTIONS_DEFAULT_VALUES_H_

namespace icnet {

namespace transport {

namespace default_values {

const int interest_lifetime = 1000;                         // milliseconds
const int content_object_expiry_time = 50000;               // milliseconds -> 50 seconds
const int content_object_packet_size = 1500;                // The ethernet MTU
const int producer_socket_input_buffer_size = 150000;      // Interests
const int producer_socket_output_buffer_size = 150000;      // Content Object
const int default_buffer_size = 8096 * 8 * 2;
const int signature_size = 260;                             // bytes
const int key_locator_size = 60;                              // bytes
const int limit_guard = 80;                                 // bytes
const int min_window_size = 1;                              // Interests
const int max_window_size = 128000;                         // Interests
const int digest_size = 34;                                 // bytes
const int max_out_of_order_segments = 3;                     // content object

// RAAQM
const int sample_number = 30;
const double gamma_value = 1;
const double beta_value = 0.8;
const double drop_factor = 0.2;
const double minimum_drop_probability = 0.00001;
const int path_id = 0;
const double rate_alpha = 0.8;

// Vegas
const double alpha = 1 / 8;
const double beta = 1 / 4;
const uint16_t k = 4;
const std::chrono::milliseconds clock_granularity = std::chrono::milliseconds(100);

// maximum allowed values
const int transport_protocol_min_retransmissions = 0;
const int transport_protocol_max_retransmissions = 128;
const int max_content_object_size = 8096;

} // end namespace default_values

} // end namespace transport

} // end namespace icnet

#endif // ICNET_SOCKET_OPTIONS_DEFAULT_VALUES_H_
