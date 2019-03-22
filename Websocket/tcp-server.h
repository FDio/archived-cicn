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

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <functional>
#include <iostream>
#include <asio.hpp>

typedef std::function<std::string(const uint8_t*,
                                  std::size_t)> HandlerFunction;

class TcpServer
{
public:
  explicit
  TcpServer(unsigned short port, long read_timeout = 5);

  void
  setHandler(const HandlerFunction &handler);

  ~TcpServer();

  void
  start();

private:

  void
  accept();

  void
  processIncomingData(std::shared_ptr<asio::ip::tcp::socket> socket);

  std::shared_ptr<asio::steady_timer>
  set_timeout_on_socket(std::shared_ptr<asio::ip::tcp::socket> socket, long seconds);

  unsigned short port;
  asio::io_service io_service;
  asio::ip::tcp::acceptor acceptor;
  long read_timeout;
  HandlerFunction handler;
};


#endif //TCP_SERVER_H
