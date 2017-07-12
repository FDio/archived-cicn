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

#include "icnet_http_request.h"
#include "icnet_utils_uri.h"

namespace icnet {

namespace http {

static std::map<HTTPMethod, std::string> method_map = {
    {GET, "GET"},
    {POST, "POST"},
    {PUT, "PUT"},
    {PATCH, "PATCH"},
    {DELETE, "DELETE"},
};

//std::map<HTTPMethod, std::string> method_map

HTTPRequest::HTTPRequest(HTTPMethod method, std::string &url, HTTPHeaders &headers, HTTPPayload &payload) {
  utils::Uri uri;
  uri.parse(url);

  path_ = uri.getPath();
  query_string_ = uri.getQueryString();
  protocol_ = uri.getProtocol();
  locator_ = uri.getLocator();
  port_ = uri.getPort();

  headers_ = headers;
  payload_ = payload;

  std::transform(locator_.begin(),
                 locator_.end(),
                 locator_.begin(),
                 ::tolower);

  std::transform(protocol_.begin(),
                 protocol_.end(),
                 protocol_.begin(),
                 ::tolower);

  std::stringstream stream;
  stream << method_map[method] << " " << uri.getPath() << " HTTP/" << HTTP_VERSION << "\r\n";
  for (auto &item : headers) {
    stream << item.first << ": " << item.second << "\r\n";
  }
  stream << "\r\n";

  if (payload.size() > 0) {
    stream << payload.data();
  }

  request_string_ = stream.str();
}

std::string &HTTPRequest::getPort() {
  return port_;
}

std::string &HTTPRequest::getLocator() {
  return locator_;
}

std::string &HTTPRequest::getProtocol() {
  return protocol_;
}

std::string &HTTPRequest::getPath() {
  return path_;
}

std::string &HTTPRequest::getQueryString() {
  return query_string_;
}

HTTPHeaders &HTTPRequest::getHeaders() {
  return headers_;
}

HTTPPayload &HTTPRequest::getPayload() {
  return payload_;
}

std::string &HTTPRequest::getRequestString() {
  return request_string_;
}

}

}