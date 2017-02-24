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

#include "connection-pool.h"

namespace ns3
{
namespace emulator
{

ConnectionPool::ConnectionPool (unsigned short tcpPort, unsigned short websocketPort)
//      : m_tcpControlServer(tcpPort)
    : m_webSocketControlServer (tcpPort)
{
}

ConnectionPool &ConnectionPool::startListeners (const HandlerFunction &handler)
{
//  m_tcpControlServer.setHandler(handler);
  m_webSocketControlServer.setHandler (handler);

//  m_tcpConnectionHandle = std::async(std::launch::async, [this](){
//
//    this->m_tcpControlServer.start();
//
//  });

  m_webSocketHandle = std::async (std::launch::async, [this] ()
  {

    this->m_webSocketControlServer.start ();

  });

  return *this;
}

ConnectionPool &ConnectionPool::processEvents ()
{
//  m_tcpConnectionHandle.get();
  m_webSocketHandle.get ();
}

} // End namespace emulator

} // End namespace ns3