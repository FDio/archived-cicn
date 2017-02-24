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

#ifndef WIFI_EMULATOR_WEBSOCKETSERVER_H
#define WIFI_EMULATOR_WEBSOCKETSERVER_H

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

typedef websocketpp::server<websocketpp::config::asio> Server;
typedef Server::message_ptr message_ptr;

typedef std::function<void (Server *, websocketpp::connection_hdl, message_ptr, const uint8_t *, std::size_t)> HandlerFunction;

using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

class WebSocketServer {
 public:
  explicit WebSocketServer (unsigned short port);

  void setHandler (const HandlerFunction &handler);

  ~WebSocketServer ()
  {
  };

  void start ();

 private:
  void onMessage (Server *s, websocketpp::connection_hdl hdl, message_ptr msg);

  Server server;

  HandlerFunction handler;

  volatile bool m_isRunning;
};

#endif //WIFI_EMULATOR_WEBSOCKETSERVER_H
