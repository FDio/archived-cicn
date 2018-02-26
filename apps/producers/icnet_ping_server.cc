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

namespace icnet {

namespace transport {

class CallbackContainer {
 public:
  CallbackContainer(const std::string &prefix,
                    uint32_t object_size)
      : buffer_(object_size, 0xFF),
        content_object_(std::make_shared<ContentObject>(Name(prefix), (const uint8_t *) buffer_.data(), buffer_.size())) {
    content_object_->setExpiryTime(0);
  }

  void processInterest(ProducerSocket &p, const Interest &interest) {

    std::cout << "<<< received interest " << interest.getName() << std::endl;
    content_object_->setName(interest.getName());
    std::cout << ">>> send object " << content_object_->getName() << std::endl;
    std::cout << std::endl;

    p.produce(*content_object_);
  }

 private:
  std::string buffer_;
  std::shared_ptr<ContentObject> content_object_;
};

void help(char * program_name) {
  std::cout << "usage: " << program_name <<" [options]" << " icn-name" << std::endl;
  std::cout << "PING options" << std::endl;
  std::cout << "-s <val>  object content size (default 64B)" << std::endl;
  std::cout << "OUTPUT options" << std::endl;
  std::cout << "-H        prints this message" << std::endl;
}

int main(int argc, char **argv) {
  std::string name_prefix = "ccnx:/ipingserver";
  bool daemon = false;
  uint32_t object_size = 64;
  uint8_t ttl = 64;

  int opt;
  while ((opt = getopt(argc, argv, "s:t:dH")) != -1) {
    switch (opt) {
      case 's':
        object_size = uint32_t(std::stoul(optarg));
        break;
      case 't':
        ttl = (uint8_t) std::stoi(optarg);
        break;
      case 'd':
        daemon = true;
        break;
      case 'H':
      default:
        help(argv[0]);
        exit(EXIT_FAILURE);
    }
  }

  if (argv[optind] != nullptr) {
    name_prefix = argv[optind];
  } else {
    help(argv[0]);
    exit(EXIT_FAILURE);
  }

  if (daemon) {
    utils::Daemonizator::daemonize();
  }

  if (object_size > 1350)
    object_size = 1350;

  std::cout << "Using prefix [" << name_prefix << "]" << std::endl;

  CallbackContainer stubs(name_prefix, object_size);

  boost::asio::io_service io_service;

  ProducerSocket p(Name(name_prefix.c_str()));
  p.setSocketOption(GeneralTransportOptions::MAKE_MANIFEST, false);

  //  setting callbacks
  p.setSocketOption(ProducerCallbacksOptions::INTEREST_INPUT,
                    (ProducerInterestCallback) bind(&CallbackContainer::processInterest,
                                                    &stubs,
                                                    std::placeholders::_1,
                                                    std::placeholders::_2));

  p.attach();

  p.serveForever();

  return 0;
}

} // end namespace transport

} // end namespace icnet

int main(int argc, char **argv) {
  return icnet::transport::main(argc, argv);
}

