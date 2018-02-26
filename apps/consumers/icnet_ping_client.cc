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

#include "icnet_ccnx_portal.h"
#include "icnet_ccnx_name.h"

#include <boost/asio/steady_timer.hpp>

#include <iostream>
#include <iomanip>

namespace icnet {

namespace ccnx {

namespace ping {

typedef std::map<uint64_t, uint64_t> SendTimeMap;

class Configuration {
 public:
  uint64_t interestLifetime_;
  uint64_t pingInterval_;
  uint64_t maxPing_;
  std::string name_;
  uint8_t ttl_;

  Configuration()
    : interestLifetime_(500),                            //ms
      pingInterval_(1000000),                            //us
      maxPing_(std::numeric_limits<uint64_t>::max()),    //number of interests
      name_("ccnx:/pingserver"),                         //string
      ttl_(64) {
  }
};

class Client {
 public:
  Client(Configuration &c)
    : config_(c),
      sequence_number_(0),
      sent_(0),
      received_(0),
      timedout_(0),
      duplicated_(0),
      rtt_sum_(0),
      rtt_sum2_(0),
      rtt_min_(std::numeric_limits<uint64_t>::max()),
      rtt_max_(0),
      timer_(new boost::asio::steady_timer(portal_.getIoService())),
      signals_(new boost::asio::signal_set(portal_.getIoService(), SIGINT, SIGQUIT)) {
    signals_->async_wait(std::bind(&Client::afterSignal,
                                 this,
                                 std::placeholders::_1,
                                 std::placeholders::_2));
  }

  void ping() {
    doPing();
    portal_.runEventsLoop();
  }

  void OnContentObjectCallback(const Interest &interest, const ContentObject &object) {

    uint64_t rtt = 0;

    auto it = send_timestamps_.find(interest.getName().get(-1).toSegment());

    if (it != send_timestamps_.end()) {
      rtt = std::chrono::duration_cast<std::chrono::microseconds>(
          std::chrono::steady_clock::now().time_since_epoch()).count() - it->second;

      send_timestamps_.erase(it);

      if (rtt > rtt_max_) {
        rtt_max_ = rtt;
      }

      if (rtt < rtt_min_) {
        rtt_min_ = rtt;
      }

      rtt_sum_ += rtt;
      rtt_sum2_ += rtt * rtt;
    } else {
      // Duplicated packet!!!!
      duplicated_++;
    }

    std::cout << object.getContent().size() << " bytes content object with name ";
    std::cout << object.getName().getPrefix(-1);
    std::cout << ": ping_seq=" << object.getName().get(-1).toSegment();
    if (rtt) {
      std::cout << " time=" << std::fixed << std::setprecision(1) << float(rtt) / 1000 << " ms" << std::endl;
    } else  {
      std::cout << "DUPLICATED!!!";
      return;
    }

    received_++;
    if (sent_ >= config_.maxPing_) {
      stopPing();
    }
  }

  void OnInterestTimeoutCallback(const Interest &interest) {
    timedout_++;
    if (sent_ >= config_.maxPing_) {
      stopPing();
    }
  }

  void doPing() {

    Name interest_name(config_.name_);
    interest_name.appendSegment(sequence_number_);
    std::shared_ptr<Interest> interest = std::make_shared<Interest>(std::move(interest_name));

    interest->setInterestLifetime(uint32_t(config_.interestLifetime_));
    interest->setHopLimit(config_.ttl_);

    portal_.sendInterest(*interest,
                         std::bind(&Client::OnContentObjectCallback,
                                   this,
                                   std::placeholders::_1,
                                   std::placeholders::_2),
                         std::bind(&Client::OnInterestTimeoutCallback, this, std::placeholders::_1));

    send_timestamps_[sequence_number_] =
        std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::steady_clock::now().time_since_epoch()).count();

    sequence_number_++;
    sent_++;

    if (sent_ < config_.maxPing_) {
      this->timer_->expires_from_now(std::chrono::microseconds(config_.pingInterval_));
      this->timer_->async_wait([this](const boost::system::error_code e) {
        doPing();
      });
    }
  }

  void stopPing() {

    std::cout << std::endl;
    std::cout << "--- " << config_.name_ << " ping statistics ---" << std::endl;
    std::cout << sent_ << " packets transmitted, " << received_ << " packets received, ";
    std::cout << std::fixed << std::setprecision(1) << (1 - float(received_) / float(sent_)) * 100;
    std::cout << " % packet loss" << std::endl;

    if (received_ > 0) {
      float rtt_avg = float(rtt_sum_) / float(received_);
      float rtt2_avg = float(rtt_sum2_) / float(received_);
      float rtt_mdev = std::sqrt(rtt2_avg - rtt_avg * rtt_avg);

      std::cout << "rtt min/avg/max/mdev = ";
      std::cout << std::fixed << std::setprecision(3) << float(rtt_min_) / 1000
                << "/";
      std::cout << std::fixed << std::setprecision(3) << rtt_avg / 1000 << "/";
      std::cout << std::fixed << std::setprecision(3) << float(rtt_max_) / 1000
                << "/";
      std::cout << std::fixed << std::setprecision(3) << rtt_mdev / 1000
                << " ms";
      std::cout << std::endl;
    }

    portal_.stopEventsLoop();
  }

  void afterSignal(const boost::system::error_code& ec, int signal_number) {
    stopPing();
  }

 private:
  Portal portal_;

  Configuration config_;
  SendTimeMap send_timestamps_;
  uint64_t sequence_number_;
  uint32_t sent_;
  uint32_t received_;
  uint32_t timedout_;
  uint32_t duplicated_;
  uint64_t rtt_sum_;
  uint64_t rtt_sum2_;
  uint64_t rtt_min_;
  uint64_t rtt_max_;
  std::unique_ptr<boost::asio::steady_timer> timer_;
  std::unique_ptr<boost::asio::signal_set> signals_;
};

void help(char * program_name) {
  std::cout << "usage: " << program_name << " [options]" << " icn-name" << std::endl;
  std::cout << "PING options" << std::endl;
  std::cout << "-i <val>          ping interval in microseconds (default 10^6 us)" << std::endl;
  std::cout << "-m <val>          maximum number of pings to send (default unlimited)" << std::endl;
  std::cout << "-t <val>          set packet ttl (default 64)" << std::endl;
  //std::cout << "-j <val1> <val2>  jump <val2> sequence numbers every <val1> interests (default disabled)" << std::endl;
  std::cout << "ICN options" << std::endl;
  std::cout << "-l <val>          interest lifetime in milliseconds (default 500 ms)" << std::endl;
  std::cout << "OUTPUT options" << std::endl;
  std::cout << "-H                prints this message" << std::endl;
}

int main(int argc, char *argv[]) {

  Configuration c;
  int opt;

  while ((opt = getopt(argc, argv, "t:i:m:l:H")) != -1) {
    switch (opt) {
      case 't':
        c.ttl_ = (uint8_t) std::stoi(optarg);
        break;
      case 'i':
        c.pingInterval_ = std::stoul(optarg);
        break;
      case 'm':
        c.maxPing_ = std::stoul(optarg);
        break;
      case 'l':
        c.interestLifetime_ = std::stoul(optarg);
        break;
      case 'H':
      default:
        help(argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  if (argv[optind] == nullptr) {
    help(argv[0]);
    exit(EXIT_FAILURE);
  } else {
    c.name_ = argv[optind];
  }

  Client ping(c);
  ping.ping();

  return 0;
}

//close name spaces
}

}

}

int main(int argc, char *argv[]) {
  return icnet::ccnx::ping::main(argc, argv);
}
