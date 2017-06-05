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

class CallbackContainer {
 public:
  CallbackContainer(unsigned long download_size = 0)
      : buffer_(1400, 'X'), final_chunk_number_(0) {
    content_object_.setContent((uint8_t *) buffer_.c_str(), 1400);
    if (download_size > 0) {
      final_chunk_number_ = static_cast<uint64_t > (std::ceil(download_size / 1400.0));
    }
  }

  void processInterest(ProducerSocket &p, const Interest &interest) {
    // std::cout << "Sending response to " << interest.getName() << std::endl;
  }

  void processIncomingInterest(ProducerSocket &p, const Interest &interest) {
    content_object_.setName(Name(interest.getName().getWrappedStructure()));
    if (final_chunk_number_ > 0) {
      content_object_.setFinalChunkNumber(final_chunk_number_);
    }
    p.produce(content_object_);
  }
 private:
  ContentObject content_object_;
  std::string buffer_;
  uint64_t final_chunk_number_;
};

class Signer {
 public:
  Signer()
      : counter_(0), identity_name_(IDENTITY_NAME) {
  };

  ~Signer() {
  };

  void onPacket(ProducerSocket &p, ContentObject &contentObject) {
    counter_++;
    KeyLocator kl;
    contentObject.signWithSha256(kl);
  }

 private:
  int counter_;
  Name identity_name_;
};

int main(int argc, char **argv) {
  std::string name = "ccnx:/ccnxtest";
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
    std::cerr << "Using default name ccnx:/ccnxtest" << std::endl;
  } else {
    name = argv[optind];
  }

  if (daemon) {
    utils::Daemonizator::daemonize();
  }

  CallbackContainer stubs(download_size);

  //  Signer signer;

  std::cout << "Setting name.. " << name << std::endl;

  ProducerSocket p(Name(name.c_str()));

  p.setSocketOption(GeneralTransportOptions::MAKE_MANIFEST, false);

  //  setting callbacks
  p.setSocketOption(ProducerCallbacksOptions::INTEREST_INPUT,
                    (ProducerInterestCallback) bind(&CallbackContainer::processIncomingInterest,
                                                    &stubs,
                                                    std::placeholders::_1,
                                                    std::placeholders::_2));

  p.setSocketOption(ProducerCallbacksOptions::CACHE_MISS,
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

