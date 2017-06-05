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

#ifndef ICNET_SOCKET_H_
#define ICNET_SOCKET_H_

#include "icnet_transport_common.h"
#include "icnet_transport_socket_options_keys.h"
#include "icnet_transport_socket_options_default_values.h"
#include "icnet_transport_download_observer.h"

#define SOCKET_OPTION_GET       0
#define SOCKET_OPTION_NOT_GET   1
#define SOCKET_OPTION_SET       2
#define SOCKET_OPTION_NOT_SET   3
#define SOCKET_OPTION_DEFAULT   12345

#define VOID_HANDLER 0

namespace icnet {

namespace transport {

class ConsumerSocket;
class ProducerSocket;

typedef ccnx::Interest Interest;
typedef ccnx::ContentObject ContentObject;
typedef ccnx::Name Name;
typedef ccnx::Manifest Manifest;
typedef ccnx::Portal Portal;
typedef ccnx::KeyLocator KeyLocator;
typedef ccnx::Segment Segment;
typedef ccnx::PayloadType PayloadType;
typedef ccnx::Array Array;

typedef std::function<void(ConsumerSocket &, const Interest &)> ConsumerInterestCallback;
typedef std::function<void(ConsumerSocket &, std::vector<uint8_t> &&)> ConsumerContentCallback;
typedef std::function<void(ConsumerSocket &, const ContentObject &)> ConsumerContentObjectCallback;
typedef std::function<bool(ConsumerSocket &, const ContentObject &)> ConsumerContentObjectVerificationCallback;
typedef std::function<void(ConsumerSocket &, const Manifest &)> ConsumerManifestCallback;
typedef std::function<void(ProducerSocket &, ContentObject &)> ProducerContentObjectCallback;
typedef std::function<void(ProducerSocket &, const Interest &)> ProducerInterestCallback;

class Socket {
 public:

  virtual int setSocketOption(int socket_option_key, int socket_option_value) = 0;

  virtual int setSocketOption(int socket_option_key, double socket_option_value) = 0;

  virtual int setSocketOption(int socket_option_key, size_t socket_option_value) = 0;

  virtual int setSocketOption(int socket_option_key, bool socket_option_value) = 0;

  virtual int setSocketOption(int socket_option_key, Name socket_option_value) = 0;

  virtual int setSocketOption(int socket_option_key, ProducerContentObjectCallback socket_option_value) = 0;

  virtual int setSocketOption(int socket_option_key, ProducerInterestCallback socket_option_value) = 0;

  virtual int setSocketOption(int socket_option_key, ConsumerContentObjectVerificationCallback socket_option_value) = 0;

  virtual int setSocketOption(int socket_option_key, ConsumerContentObjectCallback socket_option_value) = 0;

  virtual int setSocketOption(int socket_option_key, ConsumerInterestCallback socket_option_value) = 0;

  virtual int setSocketOption(int socket_option_key, ConsumerContentCallback socket_option_value) = 0;

  virtual int setSocketOption(int socket_option_key, ConsumerManifestCallback socket_option_value) = 0;

  virtual int setSocketOption(int socket_option_key, KeyLocator socket_option_value) = 0;

  virtual int setSocketOption(int socket_option_key, IcnObserver *socket_option_value) = 0;

  virtual int getSocketOption(int socket_option_key, int &socket_option_value) = 0;

  virtual int getSocketOption(int socket_option_key, double &socket_option_value) = 0;

  virtual int getSocketOption(int socket_option_key, size_t &socket_option_value) = 0;

  virtual int getSocketOption(int socket_option_key, bool &socket_option_value) = 0;

  virtual int getSocketOption(int socket_option_key, Name &socket_option_value) = 0;

  virtual int getSocketOption(int socket_option_key, ProducerContentObjectCallback &socket_option_value) = 0;

  virtual int getSocketOption(int socket_option_key, ProducerInterestCallback &socket_option_value) = 0;

  virtual int getSocketOption(int socket_option_key,
                              ConsumerContentObjectVerificationCallback &socket_option_value) = 0;

  virtual int getSocketOption(int socket_option_key, ConsumerContentObjectCallback &socket_option_value) = 0;

  virtual int getSocketOption(int socket_option_key, ConsumerInterestCallback &socket_option_value) = 0;

  virtual int getSocketOption(int socket_option_key, ConsumerContentCallback &socket_option_value) = 0;

  virtual int getSocketOption(int socket_option_key, ConsumerManifestCallback &socket_option_value) = 0;

  virtual int getSocketOption(int socket_option_key, KeyLocator &socket_option_value) = 0;

  virtual int getSocketOption(int socket_option_key, std::shared_ptr<Portal> &socket_option_value) = 0;

  virtual int getSocketOption(int socket_option_key, IcnObserver **socket_option_value) = 0;

 protected:
  virtual ~Socket() {
  };
};

} // end namespace transport

} // namespace icnet

#endif // ICNET_SOCKET_H_
