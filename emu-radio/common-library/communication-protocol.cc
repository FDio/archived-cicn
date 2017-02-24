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

#include "communication-protocol.h"


///////////////////
//Explanation: A query can select/update 2 objects:
//  - Coordinates of a node
//  - MCS value
//
// AllowedObjectName select the generic object to update/select. Not all the action are allowed on a generic object.
// Furthermore not al the parameters of an object can be selected/updated.
// So an object has a list of attributes that is possible to update/select. Also There are filter: a node can be updated just if
// matches a certain filter. A filter is a tuple [key operand value]. Not all the operand are allowed on a key.
//////////////////

// TODO To improve. Right now just basic controls and actions

std::set<std::string> ProtocolDetails::AllowedObjectName = {"interface"};
std::set<std::string> ProtocolDetails::AllowedActions = {"update", "select", "subscribe"};
std::set<std::string> ProtocolDetails::AllowedFields = {"id", "x", "y", "rate"};
std::set<std::string> ProtocolDetails::AllowedFilters = {"id"};
std::set<std::string> ProtocolDetails::AllowedOperands = {"=="};
//std::map<std::string, std::set<std::string>> ProtocolDetails::AllowedParameters = {
//        {"node", {"x", "y", "physical-rate"}},
//};

//        {"x", {"<", "<=", "==", ">", ">=", "!="}},
//        {"y", {"<", "<=", "==", ">", ">=", "!="}},
//        {"MCS", {"<", "<=", "==", ">", ">=", "!="}}

CommunicationProtocol::CommunicationProtocol (ProtocolVersion version)
    : version (version)
{
}

bool CommunicationProtocol::checkFields (const std::string &field)
{
  if (ProtocolDetails::AllowedFields.find (field) == ProtocolDetails::AllowedFields.end ())
    {
      return false;
    }

  return true;
}

bool CommunicationProtocol::checkAction (const std::string &action)
{
  if (ProtocolDetails::AllowedActions.find (action) != ProtocolDetails::AllowedActions.end ())
    {
      return true;
    }
  else
    {
      return false;
    }
}

bool CommunicationProtocol::checkObjectName (const std::string &objectName)
{
  if (ProtocolDetails::AllowedObjectName.find (objectName) != ProtocolDetails::AllowedObjectName.end ())
    {
      return true;
    }
  else
    {
      return false;
    }
}

bool CommunicationProtocol::checkFilter (const std::vector<std::string> &filter)
{
  // TODO So far just the filter interface id == something is supported.

  if (filter.size () < 3)
    {
      std::cerr << "The format of the filter is not correct." << std::endl;
      return false;
    }

  if (ProtocolDetails::AllowedFilters.find (filter[0]) != ProtocolDetails::AllowedFilters.end ())
    {
      // The filter is supported
      if (filter[0] == "id")
        {
          if (ProtocolDetails::AllowedOperands.find (filter[1]) == ProtocolDetails::AllowedOperands.end ())
            {
              std::cerr << "The operand expressed in the filter is not compatible with the key of the filter"
                        << std::endl;
              return false;
            }
        }
    }
  else
    {
      std::cerr << "The filter contains an unknown key." << std::endl;
      return false;
    }

  return true;
}

bool CommunicationProtocol::checkParameters (const std::string &parameter)
{

  if (ProtocolDetails::AllowedFields.find (parameter) == ProtocolDetails::AllowedFields.end ())
    {
      std::cerr << "The parameter specified [" << parameter << "] cannot be selected (or does not exist)." << std::endl;
      return false;
    }

  return true;
}

std::string CommunicationProtocol::evaluateFilters (const std::list<std::vector<std::string>> &filters)
{
  // Switch between operand values

  for (auto filter : filters)
    {
      if (filter[0] == "id" and filter[1] == "==")
        {
          return filter[2];
        }
      else
        {
          std::cerr << "So far just one filter is supported [==]" << std::endl;
        }
    }
  return std::string ("");
}

void
CommunicationProtocol::timerCallback (ns3::emulator::Emulator &emulator, Server *s, websocketpp::connection_hdl hdl, message_ptr msg, Query query, std::shared_ptr<boost::asio::deadline_timer> subscribeTimer)
{
  Query reply = this->makeReplyQuery (query, emulator);

  if (reply.isEmpty ())
    {
      std::cerr << "Malformed reply. Closing connection.." << std::endl;
      return;
    }

  std::cout << "SENDING: " << reply.toJsonString () << std::endl;

  try
    {
      s->send (hdl, reply.toJsonString (), msg->get_opcode ());
    }
  catch (...)
    {
      std::cerr << "Impossible to connect to the remote endpoint. Closing connection..." << std::endl;
      return;
    }

  // Reschedule the timer for 1 second in the future:
  subscribeTimer->expires_from_now (boost::posix_time::milliseconds (1000));
  subscribeTimer
      ->async_wait ([this, &emulator, s, hdl, msg, query, subscribeTimer] (const boost::system::error_code &ec)
                    {
                      if (!ec)
                        {
                          timerCallback (emulator, s, hdl, msg, query, subscribeTimer);
                        }
                    });
}

void
CommunicationProtocol::processQuery (Server *s, websocketpp::connection_hdl hdl, message_ptr msg, ns3::emulator::Emulator &emulator, Query query)
{
  std::string interface_id;

  std::cout << query.toJsonString () << std::endl;

  const std::string &action = query.getAction ();

  if (!checkAction (action))
    {
      std::cout << "Error in the action" << std::endl;
      return;
    }

  const std::string &objectName = query.getObjectName ();

  if (!checkObjectName (objectName))
    {
      std::cout << "Error in the object name" << std::endl;
      return;
    }

  // See the filter for understanding which node update
  const std::list<std::vector<std::string>> &filters = query.getFilter ();

  for (auto filter : filters)
    {
      if (!checkFilter (filter))
        {
          std::cout << "Error in the filters." << std::endl;
        }
    }

  // Evaluate the filter
  std::string station = evaluateFilters (filters);

  if (action == *ProtocolDetails::AllowedActions.find ("update"))
    {
      // See what update (so far just x and y)
      const std::map<std::string, std::string> &params = query.getParams ();

      if (objectName == *ProtocolDetails::AllowedObjectName.find ("interface"))
        {

          if (station != "")
            {

              for (auto param : params)
                {
                  if (checkParameters (param.first))
                    {

                      if (param.first == *ProtocolDetails::AllowedFields.find ("x"))
                        {
                          emulator.setStationXCoordinate (station, std::stod (param.second));
                        }
                      else if (param.first == *ProtocolDetails::AllowedFields.find ("y"))
                        {
                          emulator.setStationYCoordinate (station, std::stod (param.second));
                        }
                    }
                }
            }
        }
    }
  else if (action == *ProtocolDetails::AllowedActions.find ("select"))
    {

      Query reply = makeReplyQuery (query, emulator);
      reply.setLast (true);
      try
        {
          s->send (hdl, reply.toJsonString (), msg->get_opcode ());
        }
      catch (...)
        {
          std::cerr << "Impossible to connect to the remote endpoint. Closing connection..." << std::endl;
        }

    }
  else if (action == *ProtocolDetails::AllowedActions.find ("subscribe"))
    {
      // Schedule send each second
      std::shared_ptr<boost::asio::deadline_timer> subscribeTimer = std::make_shared<boost::asio::deadline_timer> (s->get_io_service (), boost::posix_time::milliseconds (1000));

      subscribeTimer
          ->async_wait ([this, &emulator, s, hdl, msg, query, subscribeTimer] (const boost::system::error_code &ec)
                        {
                          if (!ec)
                            {
                              timerCallback (emulator, s, hdl, msg, query, subscribeTimer);
                            }
                        });
    }
}

Query CommunicationProtocol::makeReplyQuery (const Query &request, ns3::emulator::Emulator &emulator)
{
  // See the fields to select
  const std::list<std::string> &fields = request.getFields ();
  const std::string &objectName = request.getObjectName ();
  const std::list<std::vector<std::string>> &filters = request.getFilter ();

  // Evaluate the filter
  std::string station = evaluateFilters (filters);

  if (objectName == "interface")
    {

      if (station != "")
        {

          std::string update = *ProtocolDetails::AllowedActions.find ("update");
          std::string node = *ProtocolDetails::AllowedObjectName.find ("interface");

          std::vector<std::string> parameter;
          parameter.push_back (*ProtocolDetails::AllowedFields.find ("id"));
          parameter.push_back (*ProtocolDetails::AllowedOperands.find ("=="));
          parameter.push_back (station);

          std::list<std::vector<std::string>> fltr;
          fltr.push_back (parameter);

          std::list<std::string> flds = {};

          std::map<std::string, std::string> prms;

          for (auto field : fields)
            {
              if (field == "x" || field == "*")
                {
                  double x;
                  if (emulator.getStationXCoordinate (station, &x))
                    {
                      prms["x"] = std::to_string (x);
                    }
                }
              if (field == "y" || field == "*")
                {
                  double y;
                  if (emulator.getStationYCoordinate (station, &y))
                    {
                      prms["y"] = std::to_string (y);
                    }
                }
              if (field == "rate" || field == "*")
                {
                  double physicalRate;
                  if (emulator.getTransmissionRate (station, &physicalRate))
                    {
                      prms["rate"] = std::to_string (physicalRate);
                    }
                }
              else
                {

                }
            }

          return Query (*ProtocolDetails::AllowedActions.find ("update"), *ProtocolDetails::AllowedObjectName
              .find ("interface"), fltr, prms, flds, false);
        }
    }

  // Empty reply
  return Query ();

}
