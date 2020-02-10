/*
 * DASHManager.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "DASHManager.h"

using namespace libdash::framework::input;
using namespace libdash::framework::buffer;

using namespace dash;
using namespace dash::network;
using namespace dash::mpd;
using namespace libdash::framework::mpd;

DASHManager::DASHManager(viper::managers::StreamType type, uint32_t maxCapacity, IDASHManagerObserver* stream, MPDWrapper* mpdWrapper, bool icnEnabled, double icnAlpha, bool nodecoding, float beta, float drop, std::string v6FirstWord) :
    readSegmentCount    (0),
    receiver            (NULL),
    multimediaStream    (stream),
    isRunning           (false),
    icn                 (icnEnabled),
    icnAlpha            (icnAlpha),
    noDecoding          (nodecoding),
    beta                (beta),
    drop                (drop),
    v6FirstWord         (v6FirstWord)
{

    this->buffer = new Buffer<MediaObject>(maxCapacity,libdash::framework::buffer::VIDEO);
    this->buffer->attachObserver(this);

    this->receiver  = new DASHReceiver(type, mpdWrapper, this, this->buffer, maxCapacity, this->isICN(), this->icnAlpha, this->beta, this->drop, this->v6FirstWord);
}
DASHManager::~DASHManager()
{
    this->stop();
    delete this->receiver;
    delete this->buffer;

    this->receiver = NULL;
    this->buffer   = NULL;
}

bool DASHManager::isICN()
{
    return this->icn;
}

void		DASHManager::shouldAbort()
{
    qDebug("DASH MANAGER: ABORT REQUEST");
    this->receiver->ShouldAbort();
}

bool DASHManager::start()
{
    this->receiver->SetAdaptationLogic(this->adaptationLogic);
    if (!this->receiver->Start())
        return false;

    this->isRunning = true;
    return true;
}

void DASHManager::stop()
{
    if (!this->isRunning)
        return;

    this->isRunning = false;

    this->receiver->Stop();
    this->buffer->clear();
}

uint32_t DASHManager::getPosition()
{
    return this->receiver->GetPosition();
}

void DASHManager::setLooping(bool looping)
{
    this->receiver->SetLooping(looping);
}

void DASHManager::setPosition(uint32_t segmentNumber)
{
    this->receiver->SetPosition(segmentNumber);
}

void DASHManager::setPositionInMsec(uint32_t milliSecs)
{
    this->receiver->SetPositionInMsecs(milliSecs);
}

void DASHManager::setAdaptationLogic(libdash::framework::adaptation::IAdaptationLogic *_adaptationLogic)
{
    this->adaptationLogic = _adaptationLogic;
}

void DASHManager::clear()
{
    this->buffer->clear();
}

void DASHManager::setRepresentation()
{
    this->receiver->SetRepresentation();
}

void DASHManager::enqueueRepresentation(IPeriod *period, IAdaptationSet *adaptationSet, IRepresentation *representation)
{
//    this->receiver->SetRepresentation(period, adaptationSet, representation);
}

void DASHManager::onSegmentDownloaded()
{
    this->readSegmentCount++;
}

void DASHManager::notifyStatistics(int segNum, uint32_t bitrate, int fps, uint32_t quality)
{
    this->multimediaStream->notifyStatistics(segNum, bitrate, fps, quality);
}

void DASHManager::notifyQualityDownloading(uint32_t quality)
{
    this->multimediaStream->notifyQualityDownloading(quality);
}

int DASHManager::getBufferLevel()
{
    int res =  this->multimediaStream->getBufferLevel();
    return this->multimediaStream->getBufferLevel();
}

bool DASHManager::canPush()
{
    return this->multimediaStream->canPush();
}

MediaObject* DASHManager::getSegment()
{
    return this->buffer->getFront();
}

void DASHManager::setTargetDownloadingTime(double target)
{
    this->receiver->SetTargetDownloadingTime(target);
}

void DASHManager::onBufferStateChanged(BufferType type, uint32_t fillstateInPercent, int maxC)
{
    this->multimediaStream->onSegmentBufferStateChanged(fillstateInPercent, maxC);
    if(this->adaptationLogic->isBufferBased())
        this->receiver->OnSegmentBufferStateChanged(fillstateInPercent, maxC);
}
void DASHManager::fetchMPD()
{
    this->multimediaStream->fetchMPD();
}
