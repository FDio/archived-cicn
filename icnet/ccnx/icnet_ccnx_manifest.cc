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

#include "icnet_ccnx_manifest.h"
#include <boost/property_tree/json_parser.hpp>

namespace icnet {

namespace ccnx {

Manifest::Manifest(Name &name) : ContentObject(name) {
}

std::size_t Manifest::estimateManifestSize() {
  std::size_t capacity = 0;

  //  for (auto pair : m_mapNameDigest)
  //    capacity += pair.first.size() + pair.second.data().size();

  return capacity;
}

Manifest::Manifest(ContentObject &content_object) {
  decode();
}

void Manifest::encode() {
  std::stringstream ss;
  //boost::property_tree::write_json(ss, m_mapNameDigest);

  std::string json_string = ss.str();

  ContentObject::setContent(PayloadType::MANIFEST, (uint8_t *) json_string.c_str(), json_string.size());
}

void Manifest::decode() {
  PARCBuffer *payload = ccnxContentObject_GetPayload(ccnx_content_object_);
  char *buffer = parcBuffer_ToString(payload);

  std::stringstream ss;
  ss << buffer;

  //boost::property_tree::read_json(ss, m_mapNameDigest);

  free(buffer);
}

std::string Manifest::getDigest(ContentObject &content_object) {
  //  ContentObject &ccnxData = (ContentObject &) content_object;
  //  for (auto pair : m_mapNameDigest)
  //    if (pair.second.content_object() == ccnxData.getName().toUri()) {
  //      return pair.second.content_object();
  //    }

  return std::string();
}

void Manifest::addNameToCatalogue(Name &name, uint8_t *digest, std::size_t digest_size) {
  // Name &ccnxName = (Name &) name;
  // m_mapNameDigest.put(ccnxName.toUri(), std::string((char *) digest, digest_size));
  return;
}

} // end namespace ccnx

} // end namespace icnet