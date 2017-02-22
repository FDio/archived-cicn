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

#ifndef ICNET_CCNX_LOCAL_CONNECTOR_H_
#define ICNET_CCNX_LOCAL_CONNECTOR_H_

#include "icnet_ccnx_common.h"
#include "icnet_ccnx_network_message.h"
#include "icnet_ccnx_name.h"
#include <boost/asio.hpp>
#include <deque>

extern "C" {
#include <ccnx/common/ccnx_Interest.h>
#include <ccnx/common/ccnx_ContentObject.h>
#include <parc/security/parc_Security.h>
#include <ccnx/api/ccnx_Portal/ccnx_Portal.h>
#include <ccnx/api/ccnx_Portal/ccnx_PortalRTA.h>
#include <ccnx/common/codec/schema_v1/ccnxCodecSchemaV1_PacketEncoder.h>
};

namespace icnet {

namespace ccnx {

using boost::asio::ip::tcp;
typedef std::deque<CCNxMetaMessage *> CcnxTransportMessageQueue;
typedef std::function<void(CCNxMetaMessage *)> MessageReceivedCallback;

class LocalConnector {
 public:
  LocalConnector(boost::asio::io_service &io_service,
                 std::string &ip_address,
                 std::string &port,
                 MessageReceivedCallback receive_callback,
                 std::list<Name> &name_list);

  ~LocalConnector();

  void send(CCNxMetaMessage *message);

  void bind(Name &name);

  void close();

 private:
  void doConnect();

  void doReadHeader();

  void doReadBody();

  void doWrite();

  bool checkConnected();

 private:

  void handleDeadline(const boost::system::error_code &ec);

  void startConnectionTimer();

  void tryReconnect();

  boost::asio::io_service &io_service_;
  boost::asio::ip::tcp::socket socket_;
  boost::asio::ip::tcp::resolver resolver_;
  boost::asio::ip::tcp::resolver::iterator endpoint_iterator_;
  boost::asio::deadline_timer timer_;

  TransportMessage read_msg_;
  CcnxTransportMessageQueue write_msgs_;

  bool is_connecting_;
  bool is_reconnection_;
  bool data_available_;

  MessageReceivedCallback receive_callback_;
  std::list<Name> &served_name_list_;
};

} // end namespace ccnx

} // end namespace icnet

#endif // ICNET_CCNX_LOCAL_CONNECTOR_H_
