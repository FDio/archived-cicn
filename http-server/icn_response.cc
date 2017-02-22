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

#include "icn_response.h"

namespace icn_httpserver {

IcnResponse::IcnResponse(std::shared_ptr<icnet::ProducerSocket> producer,
                         std::string ndn_name,
                         std::string ndn_path,
                         int response_id)
    : producer_(producer), ndn_name_(ndn_name), ndn_path_(ndn_path), response_id_(response_id) {
}

void IcnResponse::send(const SendCallback &callback) {
  std::size_t buffer_size = this->streambuf_.size();
  this->streambuf_.commit(this->streambuf_.size());

  this->producer_->produce(icnet::Name(/*this->ndn_name*/),
                           boost::asio::buffer_cast<const uint8_t *>(this->streambuf_.data()),
                           buffer_size,
                           this->response_id_,
                           this->is_last_);

  this->streambuf_.consume(buffer_size);

  if (callback) {
    callback(boost::system::error_code());
  }
}

} // end namespace icn_httpserver
