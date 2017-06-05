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

#include "icnet_http_server_acceptor.h"

namespace icnet {

namespace http {

void onPayload(std::shared_ptr<HTTPServerPublisher> &publisher, const uint8_t *buffer, std::size_t size) {

  char *string = (char *) buffer;
  std::cout << "Received this content:" << std::endl;
  std::cout << string << std::endl;

  std::stringstream response;
  response << "HTTP/1.0 200 OK\r\n" << "Content-Length: " << size << "\r\n\r\n" << string;
  std::string response_string = response.str();

  std::thread t([publisher, response_string]() {
    publisher->publishContent((uint8_t *) response_string.data(), response_string.size(), 0, true);
    publisher->serveClients();
  });

  t.detach();
}

int main(int argc, char **argv) {
  HTTPServerAcceptor connection(std::string("http://webserver"), onPayload);
  connection.listen(false);

  return EXIT_SUCCESS;
}

}

}

int main(int argc, char **argv) {
  return icnet::http::main(argc, argv);
}