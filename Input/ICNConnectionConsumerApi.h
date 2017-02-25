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

#ifndef QTPLAYER_INPUT_ICNCONNECTIONCONSUMERAPI_H_
#define QTPLAYER_INPUT_ICNCONNECTIONCONSUMERAPI_H_

#include <QMessageLogger>
#include "../Portable/Networking.h"
#include "IICNConnection.h"
#include "../debug.h"

#include <sys/types.h>
#include <string>
#include <stdint.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include <inttypes.h>
#include <stdlib.h>
#include <stdarg.h>
#include <algorithm>
#include <icnet/icnet_socket_consumer.h>
#include <icnet/icnet_download_observer.h>
#include <future>
#include <inttypes.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

#include "../Portable/MultiThreading.h"
#include <boost/exception/diagnostic_information.hpp>

//logging purpose
#include <chrono>
#include <stdarg.h>


namespace libdash {
namespace framework {
namespace input {

class ICNConnectionConsumerApi : public IICNConnection, public icnet::IcnObserver {
public:
    ICNConnectionConsumerApi(double alpha, float beta, float drop);
    virtual ~ICNConnectionConsumerApi();

    virtual void Init(dash::network::IChunk *chunk);

    void InitForMPD(const std::string& url);

    virtual int Read(uint8_t *data, size_t len, dash::network::IChunk *chunk);

    int Read(uint8_t *data, size_t len);

    virtual int Peek(uint8_t *data, size_t len, dash::network::IChunk *chunk);

    virtual double GetAverageDownloadingSpeed();

    virtual double GetDownloadingTime();

    void processPayload(icnet::ConsumerSocket& , const uint8_t*, size_t);

    bool onPacket(icnet::ConsumerSocket& , const icnet::ContentObject&);

    const std::vector<dash::metrics::ITCPConnection *> &GetTCPConnectionList() const;

    const std::vector<dash::metrics::IHTTPTransaction *> &GetHTTPTransactionList() const;
    virtual void SetBeta(float beta);
    virtual void SetDrop(float drop);
    virtual void notifyStats(double throughput);

private:
    float beta;
    float drop;
    uint64_t i_chunksize;
    int i_lifetime;
    int i_missed_co;
    /**< number of content objects we missed in ICNBlock */

    std::string m_name;
    icnet::ccnx::Name m_recv_name;
    icnet::ccnx::Portal m_portal;
    int m_first;
    bool m_isFinished;
    uint64_t m_nextSeg;

    double icnAlpha;
    bool icnRateBased;

    bool allow_stale;
    int sysTimeout;
    unsigned InitialMaxwindow;
    unsigned int timer;
    double drop_factor;
    double p_min;
    unsigned int gamma;
    unsigned int samples;
    unsigned int nchunks; // XXX chunks=-1 means: download the whole file!
    bool output;
    bool report_path;
    icnet::ConsumerSocket* myConsumer;
    bool res;
    std::vector<char> mdata;
    char* deezData;
    int deezDataSize;
    int datSize;
    int dataPos;
    int firstChunk;
    double  speed; // in bps
    double dnltime; //in seconds
    uint64_t sizeDownloaded;
    std::chrono::time_point<std::chrono::system_clock> m_start_time;

    std::vector<dash::metrics::ITCPConnection *> tcpConnections;
    std::vector<dash::metrics::IHTTPTransaction *> httpTransactions;
    uint64_t cumulativeBytesReceived;

    mutable CRITICAL_SECTION monitorMutex;
    mutable CONDITION_VARIABLE contentRetrieved;
};

} /* namespace input */
} /* namespace framework */
} /* namespace libdash */

#endif /* QTPLAYER_INPUT_ICNCONNECTIONCONSUMERAPI_H_ */
