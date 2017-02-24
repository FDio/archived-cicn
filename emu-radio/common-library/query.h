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

#ifndef WIFI_EMULATOR_QUERY_H
#define WIFI_EMULATOR_QUERY_H

#include <string>
#include <list>
#include <vector>
#include <map>

#include "json.h"

typedef struct QueryKeys {
  static const std::string ACTION;
  static const std::string OBJECT_NAME;
  static const std::string FILTER;
  static const std::string PARAMS;
  static const std::string FIELD_NAMES;
  static const std::string LAST;
} QueryKeys;

// for convenience
typedef nlohmann::json Json;

class Query {
 public:
  Query ();

  Query (const std::string &action, const std::string &objectName, const std::list<std::vector<std::string>> &filter, const std::map<std::string, std::string> &params, const std::list<std::string> &fields, bool last);

  static Query fromJsonString (const std::string &jsonString);

  static void removeQuotes (std::string &string);

  static std::string toJsonString (const Query &query);

  static std::string
  toJsonString (const std::string &action, const std::string &objectName, const std::list<std::vector<std::string>> &filter, const std::map<std::string, std::string> &params, const std::list<std::string> &fields, bool last);

  std::string toJsonString ();

  const std::string &getAction () const;

  void setAction (const std::string &action);

  const std::string &getObjectName () const;

  void setObjectName (const std::string &objectName);

  const std::list<std::vector<std::string>> &getFilter () const;

  void setFilter (const std::list<std::vector<std::string>> &filter);

  const std::map<std::string, std::string> &getParams () const;

  void setParams (const std::map<std::string, std::string> &params);

  const std::list<std::string> &getFields () const;

  void setFields (const std::list<std::string> &fields);

  bool isLast () const;

  void setLast (int last);

  bool isEmpty ();

 private:
  Json query;

  std::string action;
  std::string objectName;
  std::list<std::vector<std::string>> filter;
  std::map<std::string, std::string> params;
  std::list<std::string> fields;
  bool last;
};

#endif //WIFI_EMULATOR_QUERY_H
