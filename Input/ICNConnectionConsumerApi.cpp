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
using namespace icnet;

using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;

using duration_in_seconds = std::chrono::duration<double, std::ratio<1, 1> >;


namespace libdash {
namespace framework {
namespace input {
ICNConnectionConsumerApi::ICNConnectionConsumerApi(double alpha, float beta, float drop) :
    m_recv_name(ccnx::Name()),
    m_first(1),
    m_isFinished(false),
    sizeDownloaded (0),
    cumulativeBytesReceived(0),
    icnAlpha(alpha),
    beta(beta),
    drop(drop)
{
    gamma = 1;
    this->speed = 0.0;
    this->dnltime = 0.0;
    this->deezData = NULL;
    this->deezDataSize = 0;
    this->datSize = 0;
    this->dataPos = 0;
    InitializeConditionVariable (&this->contentRetrieved);
    InitializeCriticalSection   (&this->monitorMutex);

    this->myConsumer = new ConsumerSocket(ccnx::Name(), TransportProtocolAlgorithms::RAAQM);
    this->myConsumer->setSocketOption(RaaqmTransportOptions::GAMMA_VALUE, (int)gamma);

    bool configFile = false;
    //CHECK if we are not going to override the configuration file. (if !autotune)
    if(FILE *fp = fopen("/usr/etc/consumer.conf", "r"))
    {
        fclose(fp);
        configFile = true;
    }
    if(!configFile)
    {
        this->myConsumer->setSocketOption(RaaqmTransportOptions::BETA_VALUE, this->beta);
        this->myConsumer->setSocketOption(RaaqmTransportOptions::DROP_FACTOR, this->drop);
    }
    this->myConsumer->setSocketOption(RateEstimationOptions::RATE_ESTIMATION_OBSERVER, this);
    this->myConsumer->setSocketOption(ConsumerCallbacksOptions::CONTENT_RETRIEVED, (ConsumerContentCallback) bind(&ICNConnectionConsumerApi::processPayload, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
    this->myConsumer->setSocketOption(ConsumerCallbacksOptions::CONTENT_OBJECT_TO_VERIFY, (ConsumerContentObjectVerificationCallback)bind(&ICNConnectionConsumerApi::onPacket, this, std::placeholders::_1, std::placeholders::_2));
#ifdef NO_GUI
    if(this->icnAlpha != 20)
        this->icnRateBased = true;
#else
    this->icnRateBased = true;
#endif
    Debug("ICN class created\n");

}

ICNConnectionConsumerApi::~ICNConnectionConsumerApi() {
    delete this->myConsumer;
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
    m_name = "ccnx:/" + chunk->Host() + chunk->Path();
    m_isFinished = false;

    res = false;
    dataPos = 0;
    datSize = 0;
    if(this->deezData)
    {
        memset(this->deezData, 0, this->deezDataSize);
    }

}

void ICNConnectionConsumerApi::InitForMPD(const std::string& url)
{
    m_first = 1;
    sizeDownloaded = 0;

    if(url.find("//") != std::string::npos)
    {
        int pos = url.find("//");
        char* myName = (char*)malloc(strlen(url.c_str()) - 1);
        strncpy(myName, url.c_str(), pos + 1);
        strncpy(myName + pos + 1, url.c_str() + pos + 2, strlen(url.c_str()) - pos - 2);
        m_name = std::string(myName);
        free(myName);
    }
    else
    {
        m_name = url;
    }
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
        m_start_time = std::chrono::system_clock::now();

    if(res)
    {
        if(this->dataPos == this->datSize)
        {
            this->dnltime = std::chrono::duration_cast<duration_in_seconds>(std::chrono::system_clock::now() - m_start_time).count();
            if(speed == 0 || !this->icnRateBased)
                speed = (double) (sizeDownloaded * 8 / this->dnltime);
            cumulativeBytesReceived += sizeDownloaded;
            Debug("ICN_Connection:\tFINISHED DOWNLOADING %s Average_DL: %f size: %lu cumulative: %lu Throughput: %f\n", m_name.c_str(), speed, sizeDownloaded, cumulativeBytesReceived, (double) (sizeDownloaded * 8 / this->dnltime));
            return 0;
        }
        if((this->datSize - this->dataPos) > (int)len)
        {
            memcpy(data, this->deezData + this->dataPos, len);
            this->dataPos += len;
            sizeDownloaded += len;
            return len;
        }
        else
        {
            assert(this->datSize - this->dataPos > 0);
            memcpy(data, this->deezData + this->dataPos, this->datSize - this->dataPos);
            int temp = this->datSize - this->dataPos;
            this->dataPos += this->datSize - this->dataPos;
            sizeDownloaded += temp;
            return temp;
        }
    }

    Debug("will consume: %s\n", m_name.c_str());
    this->myConsumer->consume(m_name);
    EnterCriticalSection(&this->monitorMutex);

    while(this->m_isFinished == false)
        SleepConditionVariableCS(&this->contentRetrieved, &this->monitorMutex, INFINITE);

    assert(this->datSize != 0);
    this->res = true;
    LeaveCriticalSection(&this->monitorMutex);
    if(this->datSize > (int)len)
    {
        memcpy(data, this->deezData, len);
        this->dataPos += len;
        sizeDownloaded += len;
        return len;
    }
    else
    {
        memcpy(data, this->deezData, this->datSize);
        this->dataPos += this->datSize;
        sizeDownloaded += this->datSize;
        return this->datSize;
    }
}

int             ICNConnectionConsumerApi::Peek(uint8_t *data, size_t len, IChunk *chunk) {
    return -1;
}

bool   ICNConnectionConsumerApi::onPacket(ConsumerSocket& c, const ContentObject& data)
{
    return true;
}

void   ICNConnectionConsumerApi::processPayload(ConsumerSocket& c, const uint8_t* buffer, size_t bufferSize)
{
    EnterCriticalSection(&this->monitorMutex);
    if(this->deezData == NULL)
    {
        this->deezData = (char *)malloc(bufferSize*sizeof(uint8_t));
        this->deezDataSize = bufferSize;
    }
    else
    {
        if(bufferSize > this->deezDataSize)
        {
            this->deezData = (char *)realloc(this->deezData, bufferSize * (sizeof(uint8_t)));
            this->deezDataSize = bufferSize;
        }
    }
    memcpy(this->deezData, buffer, bufferSize*sizeof(uint8_t));
    this->m_isFinished = true;
    this->datSize = (int) bufferSize;
    WakeAllConditionVariable(&this->contentRetrieved);
    LeaveCriticalSection(&this->monitorMutex);
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
