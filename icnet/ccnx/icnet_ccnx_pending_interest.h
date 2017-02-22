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

#ifndef ICNET_CCNX_PENDING_INTEREST_H_
#define ICNET_CCNX_PENDING_INTEREST_H_

#include "icnet_ccnx_interest.h"
#include "icnet_ccnx_content_object.h"
#include "icnet_ccnx_name.h"

#include <boost/asio.hpp>

namespace icnet {

namespace ccnx {

typedef std::function<void(const Interest &, ContentObject &)>
OnContentObjectCallback;
typedef std::function<void( const Interest
&)>
OnInterestTimeoutCallback;
typedef std::function<void(const Name &, const Interest
&)>
OnInterestCallback;
typedef std::function<void(const boost::system::error_code &)> BoostCallback;

class PendingInterest {
 public:

  friend class Portal;

  PendingInterest(std::shared_ptr<Interest> &interest,
                  boost::asio::io_service &portal_io_service,
                  const OnContentObjectCallback &on_content_object,
                  const OnInterestTimeoutCallback &on_interest_timeout);

  ~PendingInterest();

  bool isReceived() const;

  void startCountdown(BoostCallback &cb);

  void cancelTimer();

  void setReceived();

  const std::shared_ptr<Interest> &getInterest() const;

  void setInterest(const std::shared_ptr<Interest> &interest);

  const OnContentObjectCallback &getOnDataCallback() const;

  void setOnDataCallback(const OnContentObjectCallback &on_content_object);

  const OnInterestTimeoutCallback &getOnTimeoutCallback() const;

  void setOnTimeoutCallback(const OnInterestTimeoutCallback &on_interest_timeout);

  void setReceived(bool received);

  bool isValid() const;

  void setValid(bool valid);

 private:
  std::shared_ptr<Interest> interest_;
  boost::asio::io_service &io_service_;
  boost::asio::deadline_timer timer_;

 private:
  OnContentObjectCallback on_content_object_callback_;
  OnInterestTimeoutCallback on_interest_timeout_callback_;
  bool received_;
  bool valid_;
};

} // end namespace ccnx

} // end namespace icnet

#endif // ICNET_CCNX_PENDING_INTEREST_H_
