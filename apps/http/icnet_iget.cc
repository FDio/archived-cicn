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

typedef std::chrono::time_point<std::chrono::system_clock> Time;
typedef std::chrono::milliseconds TimeDuration;

Time t1 = std::chrono::system_clock::now();

#define DEFAULT_BETA 0.99
#define DEFAULT_GAMMA 0.07

namespace icnet {

namespace http {

void processResponse(std::string &name, HTTPResponse &&response) {

  auto &payload = response.getPayload();

  std::string filename = name.substr(1 + name.find_last_of("/"));
  std::cout << "Saving to: " << filename << " " << payload.size() / 1024 << "kB" << std::endl;
  Time t3 = std::chrono::system_clock::now();;
  std::ofstream file(filename.c_str(), std::ofstream::binary);
  file.write((char *) payload.data(), payload.size());
  file.close();
  Time t2 = std::chrono::system_clock::now();;
  TimeDuration dt = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);
  TimeDuration dt3 = std::chrono::duration_cast<std::chrono::milliseconds>(t3 - t1);
  long msec = dt.count();
  long msec3 = dt3.count();
  std::cout << "Elapsed Time: " << msec / 1000.0 << " seconds -- " << payload.size() * 8 / msec / 1000.0
            << "[Mbps] -- " << payload.size() * 8 / msec3 / 1000.0 << "[Mbps]" << std::endl;

}

int main(int argc, char **argv) {

  std::string name("http://webserver/sintel/mpd");

  if (argv[optind] == 0) {
    std::cerr << "Using default name http://webserver/sintel/mpd" << std::endl;
  } else {
    name = argv[optind];
  }

  HTTPClientConnection connection;
  connection.get(name);
  processResponse(name, connection.response());

  return EXIT_SUCCESS;
}

} // end namespace http

} // end namespace icnet

int main(int argc, char **argv) {
  return icnet::http::main(argc, argv);
}