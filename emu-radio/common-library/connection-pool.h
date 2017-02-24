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

#ifndef WIFI_EMULATOR_CONTROLPROTOCOL_H
#define WIFI_EMULATOR_CONTROLPROTOCOL_H

#include <future>
//#include "tcp-server.h"
#include "websocket-server.h"
#include <unordered_map>

namespace ns3
{
namespace emulator
{

class ConnectionPool {
 public:
  ConnectionPool (unsigned short tcpPort = 12345, unsigned short websocketPort = 23456);

  ConnectionPool &startListeners (const HandlerFunction &handler);

  ConnectionPool &processEvents ();

 private:
  std::future<void> m_webSocketHandle;
//  std::future<void> m_tcpConnectionHandle;

//  TcpServer m_tcpControlServer;
  WebSocketServer m_webSocketControlServer;
};

} // End namespace emulator
} // End namespace ns3

#endif //WIFI_EMULATOR_CONTROLPROTOCOL_H
