/*
 * DASHReceiver.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef _WEBSOCKETSERVICE_H_
#define _WEBSOCKETSERVICE_H_


#include "../UI/GraphDataSource.h"
#include "../Portable/MultiThreading.h"
#include "connection-pool.h"
#include "query.h"
#include "communication-protocol.h"
#include <boost/algorithm/string/trim.hpp>


class WebSocketService
{
public:
    WebSocketService();
    virtual ~WebSocketService();
    void setGraphDataSource(GraphDataSource *graphDataSource);
    GraphDataSource* getGraphDataSource();
    bool start();
    void stop();
private:
    GraphDataSource     *graphDataSource;
    static void* listenWebsocket(void *receiver);
    bool                isRunning;
    THREAD_HANDLE		webSocketThread;

};


#endif /* _WEBSOCKETSERVICE_H_ */
