/*
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

#include "icnet_transport_socket_consumer.h"
#include "icnet_utils_daemonizator.h"

#define DEFAULT_BETA 0.99
#define DEFAULT_GAMMA 0.07

namespace icnet {

namespace transport {

class CallbackContainer {
 public:
  CallbackContainer()
      : work_(new boost::asio::io_service::work(io_service_)),
        handler_(std::async(std::launch::async, [this]() { io_service_.run(); })) {
    seen_manifest_segments_ = 0;
    seen_data_segments_ = 0;
    byte_counter_ = 0;
  }

  ~CallbackContainer() {
    work_.reset();
  }

  void processPayload(ConsumerSocket &c, std::vector<uint8_t> &&payload) {
    std::cout << "Content retrieved!! Size: " << payload.size() << std::endl;

    io_service_.dispatch([payload]() {
      std::ofstream file("consumer_test_file", std::ofstream::binary);
      file.write((char *) payload.data(), payload.size());
      file.close();
    });
  }

  bool verifyData(ConsumerSocket &c, const ContentObject &contentObject) {
    if (contentObject.getPayloadType() == PayloadType::DATA) {
      std::cout << "VERIFY CONTENT" << std::endl;
    } else if (contentObject.getPayloadType() == PayloadType::MANIFEST) {
      std::cout << "VERIFY MANIFEST" << std::endl;
    }

    return true;
  }

  void processLeavingInterest(ConsumerSocket &c, const Interest &interest) {
    //    std::cout << "LEAVES " << interest.getName().toUri() << std::endl;
  }

 private:
  int seen_manifest_segments_;
  int seen_data_segments_;
  int byte_counter_;
  boost::asio::io_service io_service_;
  std::shared_ptr<boost::asio::io_service::work> work_;
  std::future<void> handler_;
};

class Verificator {
 public:
  Verificator() {
  };

  ~Verificator() {
    //      m_keyChain.deleteIdentity(Name(IDENTITY_NAME));
  }

  bool onPacket(ConsumerSocket &c, const ContentObject &contentObject) {

    return true;
  }
};

int main(int argc, char *argv[]) {
  double beta = DEFAULT_BETA;
  double dropFactor = DEFAULT_GAMMA;
  bool daemon = false;
  bool rttStats = false;

  int opt;
  while ((opt = getopt(argc, argv, "b:d:DR")) != -1) {
    switch (opt) {
      case 'b':
        beta = std::stod(optarg);
        break;
      case 'd':
        dropFactor = std::stod(optarg);
        break;
      case 'D':
        daemon = true;
        break;
      case 'R':
        rttStats = true;
        break;
      default:
        exit(EXIT_FAILURE);
    }
  }

  std::string name = "ccnx:/ccnxtest";

  if (argv[optind] == 0) {
    std::cerr << "Using default name ccnx:/ccnxtest" << std::endl;
  } else {
    name = argv[optind];
  }

  if (daemon) {
    utils::Daemonizator::daemonize();
  }

  ConsumerSocket c(Name(name.c_str()), TransportProtocolAlgorithms::RAAQM);

  CallbackContainer stubs;
  Verificator verificator;

  c.setSocketOption(GeneralTransportOptions::INTEREST_LIFETIME, 1001);
  c.setSocketOption(RaaqmTransportOptions::BETA_VALUE, beta);
  c.setSocketOption(RaaqmTransportOptions::DROP_FACTOR, dropFactor);
  c.setSocketOption(GeneralTransportOptions::MAX_INTEREST_RETX, 200);
  c.setSocketOption(OtherOptions::VIRTUAL_DOWNLOAD, true);
  c.setSocketOption(RaaqmTransportOptions::RTT_STATS, rttStats);

  c.setSocketOption(ConsumerCallbacksOptions::CONTENT_OBJECT_TO_VERIFY,
                    (ConsumerContentObjectVerificationCallback) std::bind(&Verificator::onPacket,
                                                                          &verificator,
                                                                          std::placeholders::_1,
                                                                          std::placeholders::_2));

  c.setSocketOption(ConsumerCallbacksOptions::CONTENT_RETRIEVED,
                    (ConsumerContentCallback) std::bind(&CallbackContainer::processPayload,
                                                        &stubs,
                                                        std::placeholders::_1,
                                                        std::placeholders::_2));

  c.setSocketOption(ConsumerCallbacksOptions::INTEREST_OUTPUT,
                    (ConsumerInterestCallback) std::bind(&CallbackContainer::processLeavingInterest,
                                                         &stubs,
                                                         std::placeholders::_1,
                                                         std::placeholders::_2));

  c.consume(Name());

  c.stop();

  return 0;

}

} // end namespace icnet

} // end namespace transport

int main(int argc, char *argv[]) {
  return icnet::transport::main(argc, argv);
}
