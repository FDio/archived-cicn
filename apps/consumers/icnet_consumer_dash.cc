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

  void processPayload(ConsumerSocket &c, const uint8_t *buffer, size_t bufferSize) {
    std::cout << "Content retrieved!! Size: " << bufferSize << std::endl;
  }

  bool verifyData(ConsumerSocket &c, const ContentObject &contentObject) {
    if (contentObject.getContentType() == PayloadType::DATA) {
      std::cout << "VERIFY CONTENT" << std::endl;
    } else if (contentObject.getContentType() == PayloadType::MANIFEST) {
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
  }

  bool onPacket(ConsumerSocket &c, const ContentObject &contentObject) {
    return true;
  }

};

void becomeDaemon() {
  pid_t process_id = 0;
  pid_t sid = 0;

  // Create child process
  process_id = fork();

  // Indication of fork() failure
  if (process_id < 0) {
    printf("fork failed!\n");
    // Return failure in exit status
    exit(EXIT_FAILURE);
  }

  // PARENT PROCESS. Need to kill it.
  if (process_id > 0) {
    printf("process_id of child process %d \n", process_id);
    // return success in exit status
    exit(EXIT_SUCCESS);
  }

  //unmask the file mode
  umask(0);

  //set new session
  sid = setsid();
  if (sid < 0) {
    // Return failure
    exit(EXIT_FAILURE);
  }

  // Change the current working directory to root.
  chdir("/");

  // Close stdin. stdout and stderr
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  // Really start application
}

int main(int argc, char **argv) {
  double beta = DEFAULT_BETA;
  double drop_factor = DEFAULT_GAMMA;
  bool daemon = false;
  bool rtt_stats = false;
  int n_segment = 427;
  bool looping = false;

  int opt;
  while ((opt = getopt(argc, argv, "b:d:DRn:l")) != -1) {
    switch (opt) {
      case 'b':
        beta = std::stod(optarg);
        break;
      case 'd':
        drop_factor = std::stod(optarg);
        break;
      case 'D':
        daemon = true;
        break;
      case 'R':
        rtt_stats = true;
        break;
      case 'n':
        n_segment = std::stoi(optarg);
        break;
      case 'l':
        looping = true;
        break;
      default:
        exit(EXIT_FAILURE);
    }
  }

  std::string name = "ccnx:/webserver/get/sintel/18000";

  if (argv[optind] == 0) {
    std::cerr << "Using default name ccnx:/webserver/sintel/18000" << std::endl;
  } else {
    name = argv[optind];
  }

  if (daemon) {
    becomeDaemon();
  }

  ConsumerSocket c(Name(name.c_str()), TransportProtocolAlgorithms::RAAQM);

  CallbackContainer stubs;
  Verificator verificator;

  c.setSocketOption(GeneralTransportOptions::INTEREST_LIFETIME, 1001);
  c.setSocketOption(RaaqmTransportOptions::BETA_VALUE, beta);
  c.setSocketOption(RaaqmTransportOptions::DROP_FACTOR, drop_factor);
  c.setSocketOption(GeneralTransportOptions::MAX_INTEREST_RETX, 10);
  c.setSocketOption(OtherOptions::VIRTUAL_DOWNLOAD, true);
  c.setSocketOption(RaaqmTransportOptions::RTT_STATS, rtt_stats);

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

  do {
    std::stringstream ss;
    for (int i = 1; i < n_segment; i++) {
      ss << "ccnx:/seg_" << i << ".m4s";
      auto str = ss.str();
      c.consume(Name(str));
      ss.str("");
    }
  } while (looping);

  c.stop();

  return 0;

}

} // end namespace icnet

int main(int argc, char **argv) {
  return icnet::main(argc, argv);
}
