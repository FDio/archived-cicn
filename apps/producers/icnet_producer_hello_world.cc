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

#include "icnet_transport_socket_producer.h"
#include "icnet_utils_daemonizator.h"

#define IDENTITY_NAME "cisco"

namespace icnet {

namespace transport {

class IcnetProducerHelloWorld {

public:
  IcnetProducerHelloWorld(Name& prefix)
      : prefix_(prefix),
        p_(prefix_) {

    p_.setSocketOption(ProducerCallbacksOptions::INTEREST_INPUT,
                      (ProducerInterestCallback) bind(&IcnetProducerHelloWorld::processIncomingInterest,
                                                      this,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2));

    p_.setSocketOption(ProducerCallbacksOptions::CACHE_MISS,
                      (ProducerInterestCallback) bind(&IcnetProducerHelloWorld::processInterest,
                                                      this,
                                                      std::placeholders::_1,
                                                      std::placeholders::_2));

  }

  void publish_content(Name name, uint8_t *buffer, std::size_t size) {
    p_.produce(name, buffer, size);
  }

  void run() {
    p_.attach();
    p_.serveForever();
  }

private:
  void processIncomingInterest(ProducerSocket &p, const Interest &interest) {
    // A new interest has been received!
    std::cout << "Received interest with name " << interest.getName() << std::endl;
  }

  void processInterest(ProducerSocket &p, const Interest &interest) {
    // The received interest did not hit any content object in the cache!
    std::cout << "The interest with name " << interest.getName() << " cannot be satisfied!" << std::endl;
  }

private:
  Name prefix_;
  ProducerSocket p_;
};

int main(int argc, char **argv) {
  std::string name = "ccnx:/helloworld";
  unsigned long download_size = 0;
  bool daemon = false;

  int opt;
  while ((opt = getopt(argc, argv, "Ds:")) != -1) {

    switch (opt) {
      case 'D':
        daemon = true;
        break;
      case 's':
        download_size = std::stoul(optarg);
        break;
      default:
        exit(EXIT_FAILURE);
    }
  }

  if (argv[optind] == 0) {
    std::cerr << "Using default name ccnx:/helloworld" << std::endl;
  } else {
    name = argv[optind];
  }

  if (daemon) {
    utils::Daemonizator::daemonize();
  }
  
  Name n(name.c_str());
  std::string content(10000, 'A');

  IcnetProducerHelloWorld producer(n);
  producer.publish_content(n, (uint8_t *)content.data(), content.size());
  producer.run();

  return 0;
}

} // end namespace transport

} // end namespace icnet

int main(int argc, char **argv) {
  return icnet::transport::main(argc, argv);
}

