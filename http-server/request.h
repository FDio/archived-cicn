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

#pragma once

#include "common.h"
#include "content.h"

namespace icn_httpserver {

class iequal_to {
 public:
  bool operator()(const std::string &key1, const std::string &key2) const {
    return boost::algorithm::iequals(key1, key2);
  }
};

class ihash {
 public:
  size_t operator()(const std::string &key) const {
    std::size_t seed = 0;
    for (auto &c: key)
      boost::hash_combine(seed, std::tolower(c));
    return seed;
  }
};

class Request {
 public:

  Request();

  virtual void read_remote_endpoint_data(socket_type &socket) {
  };

  virtual ~Request() = default;

  const std::string &getMethod() const;

  void setMethod(const std::string &method);

  const std::string &getPath() const;

  void setPath(const std::string &path);

  const std::string &getHttp_version() const;

  void setHttp_version(const std::string &http_version);

  std::unordered_multimap<std::string, std::string, ihash, iequal_to> &getHeader();

  boost::asio::streambuf &getStreambuf() {
    return streambuf_;
  }

  Content &getContent();

  const boost::smatch &getPath_match() const;

  void setPath_match(const boost::smatch &path_match);

 protected:
  std::string method_, path_, http_version_;
  Content content_;
  std::unordered_multimap<std::string, std::string, ihash, iequal_to> header_;
  boost::smatch path_match_;
  boost::asio::streambuf streambuf_;
};

} // end namespace icn_httpserver
