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

#ifndef ICNET_CCNX_PORTAL_H_
#define ICNET_CCNX_PORTAL_H_

#include "icnet_ccnx_common.h"

#include <unordered_map>
#include <memory>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio.hpp>
#include <future>

extern "C" {
#include <ccnx/api/ccnx_Portal/ccnx_Portal.h>
#include <ccnx/api/ccnx_Portal/ccnx_PortalRTA.h>
#include <ccnx/api/control/cpi_Acks.h>
#include <ccnx/common/ccnx_ContentObject.h>
#include <parc/security/parc_Security.h>
#include <parc/security/parc_IdentityFile.h>
#include <parc/security/parc_Pkcs12KeyStore.h>
#include <parc/algol/parc_Memory.h>
};

#include "icnet_ccnx_interest.h"
#include "icnet_ccnx_pending_interest.h"
#include "icnet_ccnx_name.h"
#include "icnet_ccnx_content_object.h"
#include "icnet_ccnx_local_connector.h"

namespace icnet {

namespace ccnx {

typedef std::unordered_map<Name, std::unique_ptr<PendingInterest>> PendingInterestHashTable;
typedef uint64_t PendingInterestId;
typedef CCNxMetaMessage CCNxMetaMessageStructure;

class Portal : public std::enable_shared_from_this<Portal> // : public api::Face
{
 public:
  Portal(std::string forwarder_ip_address = "127.0.0.1", std::string forwarder_port = "9695");

  ~Portal();

  void sendInterest(const Interest &interest,
                    const OnContentObjectCallback &on_content_object_callback,
                    const OnInterestTimeoutCallback &on_interest_timeout_callback);

  void bind(Name &name, const OnInterestCallback &on_interest_callback);

  void runEventsLoop();

  void sendContentObject(const ContentObject &content_object);

  void stopEventsLoop();

  void clear();

  boost::asio::io_service &getIoService();

 private:

  void processIncomingMessages(CCNxMetaMessageStructure *response);

  void processInterest(CCNxMetaMessage *response);

  void processContentObject(CCNxMetaMessage *response);

  void processControlMessage(CCNxMetaMessage *response);

  volatile bool is_running_;
  bool clear_;

  boost::asio::io_service io_service_;

  std::shared_ptr<boost::asio::io_service::work> work_;

  PendingInterestHashTable pending_interest_hash_table_;

  OnInterestCallback on_interest_callback_;
  std::list<Name> served_name_list_;

  LocalConnector connector_;
};

} // end namespace ccnx

} // end namespace icnet

#endif // ICNET_CCNX_PORTAL_H_
