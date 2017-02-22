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

#include "icnet_socket_consumer.h"

typedef std::chrono::time_point<std::chrono::system_clock> Time;
typedef std::chrono::milliseconds TimeDuration;

Time t1 = std::chrono::system_clock::now();

#define DEFAULT_BETA 0.99
#define DEFAULT_GAMMA 0.07

namespace icnet {

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

  void processPayload(ConsumerSocket &c, const uint8_t *buffer, size_t buffer_size) {
    Name m_name;
    c.getSocketOption(GeneralTransportOptions::NAME_PREFIX, m_name);
    std::string filename = m_name.toString().substr(1 + m_name.toString().find_last_of("/"));
    io_service_.dispatch([buffer, buffer_size, filename]() {
      std::cout << "Saving to: " << filename << " " << buffer_size / 1024 << "kB" << std::endl;
      Time t3 = std::chrono::system_clock::now();;
      std::ofstream file(filename.c_str(), std::ofstream::binary);
      file.write((char *) buffer, buffer_size);
      file.close();
      Time t2 = std::chrono::system_clock::now();;
      TimeDuration dt = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
      TimeDuration dt3 = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t1);
      long msec = dt.count();
      long msec3 = dt3.count();
      std::cout << "Elapsed Time: " << msec / 1000.0 << " seconds -- " << buffer_size * 8 / msec / 1000.0
                << "[Mbps] -- " << buffer_size * 8 / msec3 / 1000.0 << "[Mbps]" << std::endl;
    });
  }

  bool verifyData(ConsumerSocket &c, const ContentObject &content_object) {
    if (content_object.getContentType() == PayloadType::DATA) {
      std::cout << "VERIFY CONTENT" << std::endl;
    } else if (content_object.getContentType() == PayloadType::MANIFEST) {
      std::cout << "VERIFY MANIFEST" << std::endl;
    }

    return true;
  }

  void processLeavingInterest(ConsumerSocket &c, const Interest &interest) {
    //    std::cout << "OUTPUT: " << interest.getName() << std::endl;
  }

 private:
  int seen_manifest_segments_;
  int seen_data_segments_;
  int byte_counter_;
  boost::asio::io_service io_service_;
  std::shared_ptr<boost::asio::io_service::work> work_;
  std::future<void> handler_;
};

/*
 *  The client signature verification is currently being reworked with the new API.
 *  The implementation is disabled for the moment.
 */

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

int main(int argc, char **argv) {

  std::string url = "";
  std::string locator = "";
  std::string path = "";
  std::string name = "ccnx:/locator/get/path";
  size_t found = 0;
  size_t path_begin = 0;

  if (argv[optind] == 0) {
    std::cerr << "Missing URL" << std::endl;
    return 0;
  } else {
    url = argv[optind];
    std::cout << "Iget " << url << std::endl;
  }

  found = url.find("//");
  path_begin = url.find('/', found + 2);
  locator = url.substr(found + 2, path_begin - (found + 2));
  path = url.substr(path_begin, std::string::npos);
  std::cout << "locator " << locator << std::endl;
  std::cout << "path " << path << std::endl;
  name = "ccnx:/" + locator + "/get" + path;
  std::cout << "Iget ccnx name: " << name << std::endl;

  ConsumerSocket c(Name(name.c_str()), TransportProtocolAlgorithms::RAAQM);
  CallbackContainer stubs;
  Verificator verificator;

  c.setSocketOption(ConsumerCallbacksOptions::CONTENT_OBJECT_TO_VERIFY,
                    (ConsumerContentObjectVerificationCallback) std::bind(&Verificator::onPacket,
                                                                          &verificator,
                                                                          std::placeholders::_1,
                                                                          std::placeholders::_2));
  c.setSocketOption(ConsumerCallbacksOptions::CONTENT_RETRIEVED,
                    (ConsumerContentCallback) std::bind(&CallbackContainer::processPayload,
                                                        &stubs,
                                                        std::placeholders::_1,
                                                        std::placeholders::_2,
                                                        std::placeholders::_3));
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

int main(int argc, char **argv) {
  return icnet::main(argc, argv);
}
