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

#ifndef ICNET_TRANSPORT_PROTOCOL_H_
#define ICNET_TRANSPORT_PROTOCOL_H_

#include "icnet_transport_socket.h"

namespace icnet {

namespace transport {

class TransportProtocol {
 public:
  TransportProtocol(Socket *icn_socket);

  void updatePortal();

  bool isRunning();

  virtual void start() = 0;

  virtual void stop() = 0;

 protected:
  Socket *socket_;
  std::shared_ptr<Portal> portal_;
  bool is_running_;
};

} // end namespace transport

} // end namespace icnet

#endif // ICNET_TRANSPORT_PROTOCOL_H_
