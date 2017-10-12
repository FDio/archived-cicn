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

#include "icnet_http_client_connection.h"
#include "icnet_utils_daemonizator.h"

namespace icnet {

namespace http {

void usage(int argc, char **argv) {
  std::cout << "Usage:" << std::endl;
  std::cout << argv[0] << " [-D] [-n nbr_segments] " << "[URL]" << std::endl;
  exit(EXIT_FAILURE);
}

int main(int argc, char **argv) {

  std::string name("http://webserver/sintel/18000");
  uint32_t n_segment = 300;
  bool daemon = false;

  int opt;
  while ((opt = getopt(argc, argv, "Dn:h")) != -1) {
    switch (opt) {
      case 'D':
        daemon = true;
        break;
      case 'n':
        n_segment = (uint32_t) atoi(optarg);
        break;
      case 'h':
      default:
        usage(argc, argv);
    }
  }

  if (argv[optind] == 0) {
    std::cerr << "Using default name " << name << std::endl;
  } else {
    name = argv[optind];
  }

  if (daemon) {
    utils::Daemonizator::daemonize();
  }

  HTTPClientConnection connection;
  HTTPResponse response;

  std::stringstream ss;
  for (uint32_t i = 1; i < n_segment; i++) {
    ss << name;
    ss << "/seg_" << i << ".m4s";
    auto str = ss.str();
    connection.get(str);
    response = connection.response();
    std::cout << "SIZE: " << response.size() << std::endl;
    std::cout << (char *) response.data() << std::endl;
    ss.str("");
  }

  return EXIT_SUCCESS;
}

}

}

int main(int argc, char **argv) {
  return icnet::http::main(argc, argv);
}
