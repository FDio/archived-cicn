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

#ifndef HTTP_CLIENT_H_
#define HTTP_CLIENT_H_

#include <string>

class HTTPClient {
 public:
  HTTPClient();
  ~HTTPClient();
  /**
   * Download a file using HTTP GET and store in in a std::string
   * @param url The URL to download
   * @return The download result
   */
  bool download(const std::string& url, std::ostream& out);
 private:
  void* curl_;
};

#endif  // HTTP_CLIENT_H_