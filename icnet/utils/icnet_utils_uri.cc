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

#include "icnet_utils_uri.h"
#include "icnet_errors_runtime_exception.h"

namespace utils {

Uri::Uri() {

}

Uri &Uri::parse(const std::string &uri) {
  if (uri.length() == 0) {
    throw errors::RuntimeException("Malformed URI.");
  }

  iterator_t uriEnd = uri.end();

  // get query start
  iterator_t queryStart = std::find(uri.begin(), uriEnd, '?');

  // protocol
  iterator_t protocolStart = uri.begin();
  iterator_t protocolEnd = std::find(protocolStart, uriEnd, ':');            //"://");

  if (protocolEnd != uriEnd) {
    std::string prot = &*(protocolEnd);
    if ((prot.length() > 3) && (prot.substr(0, 3) == "://")) {
      protocol_ = std::string(protocolStart, protocolEnd);
      protocolEnd += 3;   //      ://
    } else {
      protocolEnd = uri.begin();  // no protocol
    }
  } else {
    protocolEnd = uri.begin();  // no protocol
  }
  // host
  iterator_t hostStart = protocolEnd;
  iterator_t pathStart = std::find(hostStart, uriEnd, '/');  // get pathStart

  iterator_t hostEnd = std::find(protocolEnd,
                                 (pathStart != uriEnd) ? pathStart : queryStart,
                                 ':');  // check for port

  locator_ = std::string(hostStart, hostEnd);

  // port
  if ((hostEnd != uriEnd) && ((&*(hostEnd))[0] == ':')) {
    hostEnd++;
    iterator_t portEnd = (pathStart != uriEnd) ? pathStart : queryStart;
    port_ = std::string(hostEnd, portEnd);
  }

  // path
  if (pathStart != uriEnd) {
    path_ = std::string(pathStart, queryStart);
  }
  // query
  if (queryStart != uriEnd) {
    query_string_ = std::string(queryStart, uri.end());
  }

  return *this;

}

Uri &Uri::parseProtocolAndLocator(const std::string &locator) {

  iterator_t total_end = locator.end();

  // protocol
  iterator_t protocol_start = locator.begin();
  iterator_t protocol_end = std::find(protocol_start, total_end, ':');            //"://");

  if (protocol_end != total_end) {
    std::string prot = &*(protocol_end);
    if ((prot.length() > 3) && (prot.substr(0, 3) == "://")) {
      protocol_ = std::string(protocol_start, protocol_end);
      protocol_end += 3;   //      ://
    } else {
      throw errors::RuntimeException("Malformed locator. (Missing \"://\")");
    }
  } else {
    throw errors::RuntimeException("Malformed locator. No protocol specified.");
  }

  // locator
  iterator_t host_start = protocol_end;
  iterator_t host_end = std::find(protocol_end, total_end, '/');

  if (host_start == host_end) {
    throw errors::RuntimeException("Malformed locator. Locator name is missing");
  }

  locator_ = std::string(host_start, host_end);

  return *this;
}

std::string Uri::getLocator() {
  return locator_;
}

std::string Uri::getPath() {
  return path_;
}

std::string Uri::getPort() {
  return port_;
}

std::string Uri::getProtocol() {
  return protocol_;
}

std::string Uri::getQueryString() {
  return query_string_;
}

} // end namespace utils