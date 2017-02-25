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

#include "WebSocketService.h"
#include <stdio.h>

WebSocketService::WebSocketService()
{
    this->isRunning = false;
}
WebSocketService::~WebSocketService()
{
}

bool WebSocketService::start()
{
    if(this->isRunning)
        return false;

    this->isRunning = true;
    this->webSocketThread = createThreadPortable (listenWebsocket, this);

    if(this->webSocketThread == NULL)
    {
        this->isRunning = false;
        return false;
    }
    return true;
}

void WebSocketService::stop()
{
    if(!this->isRunning)
        return;

    this->isRunning = false;

    if(this->webSocketThread != NULL)
    {
        JoinThread(this->webSocketThread);
        destroyThreadPortable(this->webSocketThread);
    }
}

void* WebSocketService::listenWebsocket(void *webSocketServiceObject)
{
    WebSocketService *webSocketService = (WebSocketService *) webSocketServiceObject;
    CommunicationProtocol protocol;

    protocol.setGraphDataSource(webSocketService->getGraphDataSource());

    HandlerFunction handler = [&protocol](Server *s, websocketpp::connection_hdl hdl, message_ptr msg, const uint8_t *data, std::size_t size) {

        std::string command((char *) data, size);

        boost::trim(command);

        std::cout << command << std::endl;

        Query query = Query::fromJsonString(command);

        protocol.processQuery(s, hdl, msg, query );
    };

    ConnectionPool connPool(8999);
    connPool.startListeners(handler).processEvents();
    return NULL;
}


void WebSocketService::setGraphDataSource(GraphDataSource *graphDataSource)
{
    this->graphDataSource = graphDataSource;
}


GraphDataSource* WebSocketService::getGraphDataSource()
{
    return this->graphDataSource;
}


