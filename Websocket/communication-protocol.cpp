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

#include "websocket-server.h"
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

std::set<std::string> ProtocolDetails::AllowedObjectName = {"stats"};
std::set<std::string> ProtocolDetails::AllowedActions = { "select", "subscribe"};
std::set<std::string> ProtocolDetails::AllowedFields = {"quality", "rate", "all"};

std::function<void(const boost::system::error_code&)> CommunicationProtocol::timerCallback;

CommunicationProtocol::CommunicationProtocol(ProtocolVersion version)
    : version(version)
{
}

bool
CommunicationProtocol::checkFields(const std::string &field)
{
    if (ProtocolDetails::AllowedFields.find(field) == ProtocolDetails::AllowedFields.end()) {
        return false;
    }

    return true;
}

bool
CommunicationProtocol::checkAction(const std::string &action)
{
    if (ProtocolDetails::AllowedActions.find(action) != ProtocolDetails::AllowedActions.end()) {
        return true;
    } else {
        return false;
    }
}

bool
CommunicationProtocol::checkObjectName(const std::string &objectName)
{
    if (ProtocolDetails::AllowedObjectName.find(objectName) != ProtocolDetails::AllowedObjectName.end()) {
        return true;
    } else {
        return false;
    }
}

bool
CommunicationProtocol::checkFilter(const std::vector<std::string> &filter)
{
    // TODO So far just the filter interface id == something is supported.
    if (filter.size() < 3) {
        std::cerr << "The format of the filter is not correct." << std::endl;
        return false;
    }
    return true;
}

bool
CommunicationProtocol::checkParameters(const std::string &parameter)
{

    if (ProtocolDetails::AllowedFields.find(parameter) == ProtocolDetails::AllowedFields.end()) {
        std::cerr << "The parameter specified [" << parameter << "] cannot be selected (or does not exist)." << std::endl;
        return false;
    }

    return true;
}

std::string
CommunicationProtocol::evaluateFilters(const std::list<std::vector<std::string>> &filters)
{
    // Switch between operand values

    for (auto filter : filters) {
        if (filter[0] == "id" and filter[1] == "==") {
            return filter[2];
        } else {
            std::cerr << "So far just one filter is supported [==]" << std::endl;
        }
    }
    return std::string("");
}

void
CommunicationProtocol::processQuery(Server *s, websocketpp::connection_hdl hdl, message_ptr msg,
                                    Query query)
{
    std::string interface_id;

    std::cout << query.toJsonString() << std::endl;

    const std::string& action = query.getAction();

    if (!checkAction(action)) {
        std::cout << "Error in the action" << std::endl;
        return ;
    }

    const std::string& objectName = query.getObjectName();
    if (!checkObjectName(objectName)) {
        std::cout << "Error in the object name" << std::endl;
        return ;
    }

    // See the filter for understanding which node update
    const std::list<std::vector<std::string>> &filters = query.getFilter();

    for (auto filter : filters) {
        if (!checkFilter(filter)) {
            std::cout << "Error in the filters." << std::endl;
        }
    }

    if (action == *ProtocolDetails::AllowedActions.find("select")) {
        Query reply = makeReplyQuery(query);
        reply.setLast(true);
        try {
            s->send(hdl, reply.toJsonString(), msg->get_opcode());
        } catch (...) {
            std::cerr << "Impossible to connect to the remote endpoint. Closing connection..." << std::endl;
        }

    } else if (action == *ProtocolDetails::AllowedActions.find("subscribe")) {

        subscribeTimer = std::shared_ptr<boost::asio::deadline_timer>(new boost::asio::deadline_timer(s->get_io_service(),
                                                                                                      boost::posix_time::milliseconds(1000)));
        timerCallback = [this, s, hdl, msg, query]
                (const boost::system::error_code &ec) {
            if (!ec) {
                Query reply = this->makeReplyQuery(query);

                if (reply.isEmpty()) {
                    std::cerr << "Malformed reply. Closing connection.." << std::endl;
                    return;
                }

                std::cout << "SENDING: " << reply.toJsonString() << std::endl;

                try {
                    s->send(hdl, reply.toJsonString(), msg->get_opcode());
                } catch (...) {
                    std::cerr << "Impossible to connect to the remote endpoint. Closing connection..." << std::endl;
                    return;
                }

                subscribeTimer->expires_from_now(boost::posix_time::milliseconds(1000));
                subscribeTimer->async_wait(timerCallback);
            }
        };

        subscribeTimer->async_wait(timerCallback);
    }
}

Query
CommunicationProtocol::makeReplyQuery(const Query &request)
{
    const std::list<std::string> &fields = request.getFields();
    const std::string &objectName = request.getObjectName();
    const std::list<std::vector<std::string>> &filters = request.getFilter();

    std::string station = evaluateFilters(filters);

    if (objectName == "stats") {


        std::list<std::vector<std::string>> fltr;
        std::list<std::string> flds = {};

        std::map<std::string, std::string> prms;

        for (auto field : fields) {
            if (field == "bufferLevel" || field == "*") {
                prms["bufferLevel"] = QString::number(graphDataSource->getBufferLevel()).toStdString();
            }

            if (field == "bitRate" || field == "*")
            {
                prms["bitRate"] = QString::number(graphDataSource->getFps()).toStdString();
            }

            if (field == "quality" || field == "*")
            {
                uint32_t quality = graphDataSource->getQuality();
                if(quality == 2)
                    prms["quality"] = "LD";
                else if(quality == 6)
                    prms["quality"] = "SD";
                else if(quality == 9)
                    prms["quality"] = "HD";
                else if(quality == 12)
                    prms["quality"] = "FHD";
                else if(quality == 15)
                    prms["quality"] = "QHD";
                else if (quality == 18)
                    prms["quality"] = "UHD";
                else
                    prms["quality"] = "Unknown";
            }
        }

        return Query(*ProtocolDetails::AllowedActions.find("select"),
                     *ProtocolDetails::AllowedObjectName.find("stats"),
                     fltr, prms, flds, false);
    }
    return Query();

}

void
CommunicationProtocol::setGraphDataSource(GraphDataSource *graphDataSource)
{
    this->graphDataSource = graphDataSource;
}
