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

#ifndef ICNICPDOWNLOAD

#include "ICNConnectionConsumerApi.h"

#define DEFAULT_LIFETIME 250
#define RETRY_TIMEOUTS 5

using namespace dash;
using namespace dash::network;
using namespace dash::metrics;
//using namespace icnet;

using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

using duration_in_seconds = std::chrono::duration<double, std::ratio<1, 1> >;


namespace libdash {
namespace framework {
namespace input {
ICNConnectionConsumerApi::ICNConnectionConsumerApi(double alpha, float beta, float drop, std::string v6FirstWord) :
    m_first(1),
    m_isFinished(false),
    sizeDownloaded (0),
    cumulativeBytesReceived(0),
    icnAlpha(alpha),
    beta(beta),
    drop(drop),
    v6FirstWord(v6FirstWord)
{
    gamma = 1;
    this->speed = 0.0;
    this->dnltime = 0.0;
    this->deezData = NULL;
    this->deezDataSize = 0;
    this->datSize = 0;
    this->dataPos = 0;
    this->res = false;
    InitializeConditionVariable (&this->contentRetrieved);
    InitializeCriticalSection   (&this->monitorMutex);
    this->hTTPClientConnection = new libl4::http::HTTPClientConnection();
#ifdef HICNET
    transport::interface::ConsumerSocket &c = this->hTTPClientConnection->getConsumer();
#else
    libl4::transport::ConsumerSocket &c = this->hTTPClientConnection->getConsumer();
#endif
    bool configFile = false;
    //CHECK if we are not going to override the configuration file. (if !autotune)
    if(FILE *fp = fopen("/usr/etc/consumer.conf", "r"))
    {
        fclose(fp);
        configFile = true;
    }
#ifdef HICNET
    if(!configFile)
    {
        qDebug("beta %f, drop %f", this->beta, this->drop);

        c.setSocketOption(transport::interface::RaaqmTransportOptions::BETA_VALUE, this->beta);
        c.setSocketOption(transport::interface::RaaqmTransportOptions::DROP_FACTOR, this->drop);
    }
    c.setSocketOption(int(transport::interface::RateEstimationOptions::RATE_ESTIMATION_OBSERVER), (transport::protocol::IcnObserver * )this);
#else
    if(!configFile)
     {
         qDebug("beta %f, drop %f", this->beta, this->drop);

         c.setSocketOption(libl4::transport::RaaqmTransportOptions::BETA_VALUE, this->beta);
         c.setSocketOption(libl4::transport::RaaqmTransportOptions::DROP_FACTOR, this->drop);
     }
     c.setSocketOption(int(libl4::transport::RateEstimationOptions::RATE_ESTIMATION_OBSERVER), (libl4::transport::IcnObserver * )this);
#endif
#ifdef NO_GUI
    if(this->icnAlpha != 20)
        this->icnRateBased = true;
#else
    this->icnRateBased = true;
#endif
    Debug("ICN class created\n");

}

ICNConnectionConsumerApi::~ICNConnectionConsumerApi() {
    delete this->hTTPClientConnection;
    if(this->deezData)
    {
        free(this->deezData);
        this->deezData = NULL;
    }
    DeleteConditionVariable (&this->contentRetrieved);
    DeleteCriticalSection   (&this->monitorMutex);
}

void ICNConnectionConsumerApi::Init(IChunk *chunk) {
    Debug("ICN Connection:	STARTING\n");
    m_first = 1;
    sizeDownloaded = 0;
    m_name = chunk->AbsoluteURI().c_str();
    m_isFinished = false;

    res = false;
    Debug("ICN_Connection:\tINTIATED_to_name %s\n", m_name.c_str());
    Debug("ICN_Connection:\tSTARTING DOWNLOAD %s\n", m_name.c_str());
}

void ICNConnectionConsumerApi::InitForMPD(const std::string& url)
{
    m_first = 1;
    sizeDownloaded = 0;
    m_name = url;
    m_isFinished = false;

    res = false;
    dataPos = 0;
    datSize = 0;
    Debug("ICN_Connection:\tINTIATED_for_mpd %s\n", m_name.c_str());
}

int	ICNConnectionConsumerApi::Read(uint8_t* data, size_t len, IChunk *chunk)
{
    return this->Read(data, len);
}

int	ICNConnectionConsumerApi::Read(uint8_t *data, size_t len)
{
    if(!res)
    {
       std::map<std::string, std::string> headers = {{"Host", "localhost"},
                                                       {"User-Agent", "higet/1.0"},
                                                       {"Connection", "Keep-Alive"}};
       std::string s(m_name.c_str());
       hTTPClientConnection->get(s, headers, {}, nullptr, nullptr, this->v6FirstWord);
#ifdef HICNET
       response = hTTPClientConnection->response()->getPayload();
#else
       response  = hTTPClientConnection->response();
#endif
       this->res = true;
       this->dataPos = 0;
    }
#ifdef HICNET
    char * bytes = (char*)response->writableData();
    std::size_t size = response->length();
#else
    char *bytes = response.getPayload().data();
    std::size_t size = response.getPayload().size();
#endif

    if (size - this->dataPos > (int)len)
    {
       memcpy(data, bytes + this->dataPos, len);
       this->dataPos += len;
       return len;
    } else
    {
        memcpy(data, bytes + this->dataPos, size - this->dataPos);
        int length = size - this->dataPos;
        if (length == 0)
        {
          this->res = false;
        }
        this->dataPos = size;
        return length;
    }
}

int             ICNConnectionConsumerApi::Peek(uint8_t *data, size_t len, IChunk *chunk) {
    return -1;
}


double ICNConnectionConsumerApi::GetAverageDownloadingSpeed()
{
    Debug("ICNConnection:	DL speed is %f\n", this->speed);
    return this->speed;
}

double ICNConnectionConsumerApi::GetDownloadingTime()
{
    Debug("ICNConnection:	DL time is %f\n", this->dnltime);
    return this->dnltime;
}

const std::vector<ITCPConnection *> &ICNConnectionConsumerApi::GetTCPConnectionList() const {
    return tcpConnections;
}

const std::vector<IHTTPTransaction *> &ICNConnectionConsumerApi::GetHTTPTransactionList() const {
    return httpTransactions;
}

void ICNConnectionConsumerApi::notifyStats(double winSize)
{
    this->speed = (winSize); // * 1000000 * 1400 * 8;
    Debug("ICNConnection:\tNotificationICPDL\t%f\t%f\n", winSize, speed);
}

void ICNConnectionConsumerApi::notifyDownloadTime(double downloadingTime)
{
    //downloadingTime is in microseconds, dnltime should be in seconds
    this->dnltime = downloadingTime / 1000000; 
}

void ICNConnectionConsumerApi::SetBeta(float beta)
{
    this->beta = beta;
}


void ICNConnectionConsumerApi::SetDrop(float drop)
{
    this->drop = drop;
}

} /* namespace input */
} /* namespace framework */
} /* namespace libdash */


#endif //NDEF ICNICPDOWNLOAD
