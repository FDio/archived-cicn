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

#ifndef ICNET_CCNX_MANIFEST_H_
#define ICNET_CCNX_MANIFEST_H_

#include "icnet_ccnx_common.h"
//#include <boost/property_tree/ptree.hpp>

#include "icnet_ccnx_name.h"
#include "icnet_ccnx_content_object.h"

namespace icnet {

namespace ccnx {

class Manifest : public ContentObject // public api::Manifest
{
 public:
  Manifest(Name &name);

  Manifest(ContentObject &content_object);

  std::size_t estimateManifestSize();

  void encode();

  void decode();

  std::string getDigest(ContentObject &content_object);

  void addNameToCatalogue(Name &name, uint8_t *digest, std::size_t digest_size);

 private:
  // boost::property_tree::ptree map_name_digest_;
};

} // end namespace ccnx

} // end namespace icnet


#endif // ICNET_CCNX_MANIFEST_H_
