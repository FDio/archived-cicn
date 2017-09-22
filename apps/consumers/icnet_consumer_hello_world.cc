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

namespace icnet {

namespace transport {

class IcnetConsumerHelloWorld {
 public:
  IcnetConsumerHelloWorld()
      : c_(Name(), TransportProtocolAlgorithms::RAAQM) {
    c_.setSocketOption(GeneralTransportOptions::INTEREST_LIFETIME, 1001);
  
  c_.setSocketOption(GeneralTransportOptions::MAX_INTEREST_RETX, 25);

  c_.setSocketOption(ConsumerCallbacksOptions::CONTENT_OBJECT_TO_VERIFY,
                    (ConsumerContentObjectVerificationCallback) std::bind(&IcnetConsumerHelloWorld::verifyPacket,
                                                                          this,
                                                                          std::placeholders::_1,
                                                                          std::placeholders::_2));

  c_.setSocketOption(ConsumerCallbacksOptions::CONTENT_RETRIEVED,
                    (ConsumerContentCallback) std::bind(&IcnetConsumerHelloWorld::processContent,
                                                        this,
                                                        std::placeholders::_1,
                                                        std::placeholders::_2));

  c_.setSocketOption(ConsumerCallbacksOptions::INTEREST_OUTPUT,
                    (ConsumerInterestCallback) std::bind(&IcnetConsumerHelloWorld::processLeavingInterest,
                                                         this,
                                                         std::placeholders::_1,
                                                         std::placeholders::_2));
  }

  void run(Name name) {
    c_.consume(name);
  }

  void stop() {
    c_.stop();
  }

 private:
  bool verifyPacket(ConsumerSocket &c, const ContentObject &contentObject) {
    return true;
  }

  void processContent(ConsumerSocket &c, std::vector<uint8_t> &&payload) {
    std::cout << "Content retrieved!! Size: " << payload.size() << std::endl;

    // Save content to a file
    std::ofstream file("consumer_hello_world_file", std::ofstream::binary);
    file.write(reinterpret_cast<char*>(payload.data()), payload.size());
    file.close();
  }

  void processLeavingInterest(ConsumerSocket &c, const Interest &interest) {
    std::cout << "Sending interest with name " << interest.getName() << std::endl;
  }
 private:
  ConsumerSocket c_;
};

int main(int argc, char *argv[]) {
  bool daemon = false;

  int opt;
  while ((opt = getopt(argc, argv, "D")) != -1) {
    switch (opt) {
      case 'D':
        daemon = true;
        break;
      default:
        exit(EXIT_FAILURE);
    }
  }

  std::string name = "ccnx:/helloworld";

  if (argv[optind] == 0) {
    std::cerr << "Using default name ccnx:/helloworld" << std::endl;
  } else {
    name = argv[optind];
  }

  if (daemon) {
    utils::Daemonizator::daemonize();
  }

  IcnetConsumerHelloWorld consumer;
  consumer.run(Name(name.c_str()));
  consumer.stop();

  return 0;

}

} // end namespace icnet

} // end namespace transport

int main(int argc, char *argv[]) {
  return icnet::transport::main(argc, argv);
}
