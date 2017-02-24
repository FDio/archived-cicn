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

#include "query.h"

const std::string QueryKeys::ACTION = "action";
const std::string QueryKeys::OBJECT_NAME = "object_name";
const std::string QueryKeys::FILTER = "filter";
const std::string QueryKeys::PARAMS = "params";
const std::string QueryKeys::FIELD_NAMES = "field_names";
const std::string QueryKeys::LAST = "last";

Query::Query ()
    : query (Json::object ())
{
}

Query::Query (const std::string &action, const std::string &objectName, const std::list<std::vector<std::string>> &filter, const std::map<std::string, std::string> &params, const std::list<std::string> &fields, bool last)
{
  this->action = action;
  this->objectName = objectName;
  this->filter = filter;
  this->params = params;
  this->fields = fields;
  this->last = last;

  if (fields.size () == 0)
    {
      this->fields.push_back ("*");
    }

  query[QueryKeys::ACTION] = action;
  query[QueryKeys::OBJECT_NAME] = objectName;
  query[QueryKeys::FILTER] = Json (filter);
  query[QueryKeys::PARAMS] = Json (params);
  query[QueryKeys::FIELD_NAMES] = Json (fields);
  query[QueryKeys::LAST] = last;
}

Query Query::fromJsonString (const std::string &jsonString)
{
  Query query;
  Json jsonQuery = Json::parse (jsonString);

  std::cout << jsonQuery << std::endl;

  std::cout << jsonQuery[QueryKeys::LAST] << std::endl;

  query.setAction (jsonQuery[QueryKeys::ACTION]);
  query.setLast (jsonQuery[QueryKeys::LAST]);
  query.setObjectName (jsonQuery[QueryKeys::OBJECT_NAME]);

  Json list = jsonQuery[QueryKeys::FIELD_NAMES];

  for (Json::iterator it = list.begin (); it != list.end (); ++it)
    {
      query.fields.push_back (*it);
    }

  if (query.fields.size () == 0)
    {
      query.fields.push_back ("*");
    }

  Json list2 = jsonQuery[QueryKeys::FILTER];

  for (Json::iterator it = list2.begin (); it != list2.end (); ++it)
    {
      query.filter.push_back (*it);
    }

  Json map = jsonQuery[QueryKeys::PARAMS];

  for (Json::iterator it = map.begin (); it != map.end (); ++it)
    {
      std::cout << it.key () << " " << it.value ().dump () << std::endl;
      query.params[it.key ()] = it.value ().dump ();
    }

  query.query[QueryKeys::ACTION] = query.action;
  query.query[QueryKeys::OBJECT_NAME] = query.objectName;
  query.query[QueryKeys::FILTER] = Json (query.filter);
  query.query[QueryKeys::PARAMS] = Json (query.params);
  query.query[QueryKeys::FIELD_NAMES] = Json (query.fields);
  query.query[QueryKeys::LAST] = query.last;

  return query;
}

void Query::removeQuotes (std::string &string)
{
  std::cout << string.front () << std::endl;
  string.erase (remove (string.begin (), string.end (), '\"'), string.end ());
}

std::string Query::toJsonString (const Query &query)
{
  return query.query.dump ();
}

std::string
Query::toJsonString (const std::string &action, const std::string &objectName, const std::list<std::vector<std::string>> &filter, const std::map<std::string, std::string> &params, const std::list<std::string> &fields, bool last)
{
  Json jsonQuery;

  jsonQuery[QueryKeys::ACTION] = action;
  jsonQuery[QueryKeys::OBJECT_NAME] = objectName;
  jsonQuery[QueryKeys::FILTER] = Json (filter);
  jsonQuery[QueryKeys::PARAMS] = Json (params);
  jsonQuery[QueryKeys::FIELD_NAMES] = Json (fields);
  jsonQuery[QueryKeys::LAST] = last;

  return jsonQuery.dump ();
}

std::string Query::toJsonString ()
{
  return query.dump ();
}

const std::string &Query::getAction () const
{
  return action;
}

void Query::setAction (const std::string &action)
{
  Query::action = action;
  removeQuotes (this->action);
  query[QueryKeys::ACTION] = this->action;
}

const std::string &Query::getObjectName () const
{
  return objectName;
}

void Query::setObjectName (const std::string &objectName)
{
  Query::objectName = objectName;
  removeQuotes (this->objectName);
  query[QueryKeys::OBJECT_NAME] = this->objectName;
}

const std::list<std::vector<std::string>> &Query::getFilter () const
{
  return filter;
}

void Query::setFilter (const std::list<std::vector<std::string>> &filter)
{
  Query::filter = filter;

  for (auto f : this->filter)
    {
      for (auto field : f)
        {
          removeQuotes (field);
        }
    }

  query[QueryKeys::FILTER] = this->filter;
}

const std::map<std::string, std::string> &Query::getParams () const
{
  return params;
}

void Query::setParams (const std::map<std::string, std::string> &params)
{
  Query::params = params;
  query[QueryKeys::PARAMS] = this->params;
}

const std::list<std::string> &Query::getFields () const
{
  return fields;
}

void Query::setFields (const std::list<std::string> &fields)
{
  Query::fields = fields;

  for (auto field : this->fields)
    {
      removeQuotes (field);
    }

  query[QueryKeys::FIELD_NAMES] = this->fields;
}

bool Query::isLast () const
{
  return last;
}

void Query::setLast (int last)
{
  Query::last = last;
  query[QueryKeys::LAST] = this->last;
}

bool Query::isEmpty ()
{
  return query.empty ();
}