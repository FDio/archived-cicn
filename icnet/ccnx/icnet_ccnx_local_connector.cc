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

#ifdef __ANDROID__
#include <android/log.h>
#endif

#include "icnet_ccnx_local_connector.h"

namespace icnet {

namespace ccnx {

LocalConnector::LocalConnector(boost::asio::io_service &io_service,
                               std::string &ip_address,
                               std::string &port,
                               MessageReceivedCallback receive_callback,
                               std::list<Name> &name_list)
    : io_service_(io_service),
      socket_(io_service_),
      resolver_(io_service_),
      endpoint_iterator_(resolver_.resolve({ip_address, port})),
      timer_(io_service),
      is_connecting_(false),
      is_reconnection_(false),
      data_available_(false),
      receive_callback_(receive_callback),
      served_name_list_(name_list) {
  startConnectionTimer();
  doConnect();
}

LocalConnector::~LocalConnector() {
}

void LocalConnector::bind(Name &name) {
  CCNxControl *control = ccnxControl_CreateAddRouteToSelfRequest(name.getWrappedStructure());
  CCNxMetaMessage *message = ccnxMetaMessage_CreateFromControl(control);
  ccnxControl_Release(&control);

  send(message);

  ccnxMetaMessage_Release((CCNxMetaMessage **) &message);

}

void LocalConnector::send(CCNxMetaMessage *message) {
  CCNxMetaMessage *msg = ccnxMetaMessage_Acquire(message);

  io_service_.post([this, msg]() {
    bool write_in_progres = !write_msgs_.empty();
    write_msgs_.push_back(msg);
    if (!is_connecting_) {
      if (!write_in_progres) {
        doWrite();
      }
    } else {
      // Tell the handle connect it has data to write
      data_available_ = true;
    }
  });
}

void LocalConnector::close() {
  io_service_.post([this]() { socket_.close(); });
}

void LocalConnector::doWrite() {
  CCNxMetaMessage *message = write_msgs_.front();
  CCNxCodecNetworkBufferIoVec *network_buffer = ccnxCodecSchemaV1PacketEncoder_DictionaryEncode(message, NULL);
  const iovec *iov = ccnxCodecNetworkBufferIoVec_GetArray(network_buffer);

  boost::asio::async_write(socket_,
                           boost::asio::buffer(iov->iov_base, iov->iov_len),
                           [this, network_buffer, message](boost::system::error_code ec, std::size_t /*length*/) {
                             if (!ec) {
                               ccnxMetaMessage_Release((CCNxMetaMessage **) &message);
                               write_msgs_.pop_front();

                               if (!write_msgs_.empty()) {
                                 doWrite();
                               }
                             } else {
                               tryReconnect();
                             }

                             ccnxCodecNetworkBufferIoVec_Release((CCNxCodecNetworkBufferIoVec **) &network_buffer);

                           });

}

void LocalConnector::doReadBody() {
  boost::asio::async_read(socket_,
                          boost::asio::buffer(read_msg_.body(), read_msg_.bodyLength()),
                          boost::asio::transfer_exactly(read_msg_.bodyLength()),
                          [this](boost::system::error_code ec, std::size_t length) {
                            if (!ec) {
                              receive_callback_(read_msg_.decodeMessage());
                              doReadHeader();
                            } else {
                              tryReconnect();
                            }
                          });
}

void LocalConnector::doReadHeader() {
  boost::asio::async_read(socket_,
                          boost::asio::buffer(read_msg_.data(), TransportMessage::header_length),
                          boost::asio::transfer_exactly(TransportMessage::header_length),
                          [this](boost::system::error_code ec, std::size_t /*length*/) {
                            if (!ec) {
                              if (read_msg_.decodeHeader()) {
                                doReadBody();
                              } else {
                                std::cerr << "Decoding error" << std::endl;
                              }
                            } else {
                              tryReconnect();
                            }
                          });
}

void LocalConnector::tryReconnect() {
  if (!is_connecting_) {
#ifdef __ANDROID__
  __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "Connection lost. Trying to reconnect...\n");
#else
    std::cerr << "Connection lost. Trying to reconnect..." << std::endl;
#endif
    is_connecting_ = true;
    is_reconnection_ = true;
    io_service_.post([this]() {
      socket_.close();
      startConnectionTimer();
      doConnect();
    });
  }
}

void LocalConnector::doConnect() {
  boost::asio::async_connect(socket_,
                             endpoint_iterator_,
                             [this](boost::system::error_code ec, tcp::resolver::iterator) {
                               if (!ec) {
                                 timer_.cancel();
                                 is_connecting_ = false;
                                 doReadHeader();

                                 if (data_available_) {
                                   data_available_ = false;
                                   doWrite();
                                 }

                                 if (is_reconnection_) {
                                   is_reconnection_ = false;
#ifdef __ANDROID__
                                   __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "Connection recovered!\n");
#else
                                   std::cout << "Connection recovered!" << std::endl;
#endif
                                   for (auto &name : served_name_list_) {
                                     bind(name);
                                   }
                                 }

                               } else {
                                 sleep(1);
                                 doConnect();
                               }
                             });
}

bool LocalConnector::checkConnected() {
  return !is_connecting_;
}

void LocalConnector::startConnectionTimer() {
  timer_.expires_from_now(boost::posix_time::seconds(20));
  timer_.async_wait(std::bind(&LocalConnector::handleDeadline, this, std::placeholders::_1));
}

void LocalConnector::handleDeadline(const boost::system::error_code &ec) {
  if (!ec) {
    io_service_.post([this]() {
      socket_.close();
#ifdef __ANDROID__
      __android_log_print(ANDROID_LOG_DEBUG, "libICNet", "Error connecting. Is the forwarder running?\n");
#else
      std::cerr << "Error connecting. Is the forwarder running?" << std::endl;
#endif
      io_service_.stop();
    });
  }
}

} // end namespace ccnx

} // end namespace icnet