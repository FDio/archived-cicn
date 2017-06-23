/*
 * MultimediaStream.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "MultimediaStream.h"

using namespace viper::managers;
using namespace libdash::framework::adaptation;
using namespace libdash::framework::input;
using namespace libdash::framework::buffer;
using namespace libdash::framework::mpd;
using namespace dash::mpd;

MultimediaStream::MultimediaStream(StreamType type, MPDWrapper *mpdWrapper, uint32_t bufferSize, bool icnEnabled, double icnAlpha, bool nodecoding, float beta, float drop) :
    type		(type),
    segmentBufferSize	(bufferSize),
    dashManager		(NULL),
    mpdWrapper		(mpdWrapper),
    icn			(icnEnabled),
    icnAlpha		(icnAlpha),
    noDecoding		(nodecoding),
    beta            	(beta),
    drop            	(drop)
{
//    InitializeCriticalSection (&this->monitorMutex);
    this->init();
}
MultimediaStream::~MultimediaStream ()
{
//    DestroyCriticalSection (&this->monitorMutex);
    this->stop();
    delete this->dashManager;
}

bool MultimediaStream::isICN()
{
    return this->icn;
}

void MultimediaStream::shouldAbort()
{
    this->dashManager->shouldAbort();
}

uint32_t MultimediaStream::getPosition()
{
    return this->dashManager->getPosition();
}

void MultimediaStream::setLooping(bool looping)
{
    this->dashManager->setLooping(looping);
}

void MultimediaStream::setPosition(uint32_t segmentNumber)
{
    this->dashManager->setPosition(segmentNumber);
}

void MultimediaStream::setPositionInMsec(uint32_t milliSecs)
{
    this->dashManager->setPositionInMsec(milliSecs);
}

void MultimediaStream::init()
{
    this->dashManager   = new DASHManager(this->type, this->segmentBufferSize, this, this->mpdWrapper, this->isICN(), this->icnAlpha, this->noDecoding, this->beta, this->drop);
}

bool MultimediaStream::start()
{
    if(!this->startDownload())
        return false;

    return true;
}

bool MultimediaStream::startDownload()
{
    dashManager->setAdaptationLogic(this->logic);
    if(!dashManager->start())
        return false;

    return true;
}

void MultimediaStream::stop()
{
    this->stopDownload();
}

void MultimediaStream::stopDownload()
{
    this->dashManager->stop();
}

void MultimediaStream::clear()
{
    this->dashManager->clear();
}

void MultimediaStream::addFrame(QImage *frame)
{
}

QImage* MultimediaStream::getFrame()
{
    return NULL;
}

void MultimediaStream::attachStreamObserver(IStreamObserver *observer)
{
    this->observers.push_back(observer);
}

void MultimediaStream::setRepresentation()
{
    this->dashManager->setRepresentation();
}

void MultimediaStream::enqueueRepresentation(IPeriod *period, IAdaptationSet *adaptationSet, IRepresentation *representation)
{
    this->dashManager->enqueueRepresentation(period, adaptationSet, representation);
}

void MultimediaStream::setAdaptationLogic(libdash::framework::adaptation::IAdaptationLogic *logic)
{
    this->logic = logic;
}

void MultimediaStream::onSegmentBufferStateChanged(uint32_t fillstateInPercent, int maxC)
{
    for (size_t i = 0; i < observers.size(); i++)
        this->observers.at(i)->onSegmentBufferStateChanged(this->type, fillstateInPercent, maxC);
}

void MultimediaStream::onBufferStateChanged(BufferType type, uint32_t fillstateInPercent, int maxC)
{
    switch(type)
    {
    case libdash::framework::buffer::AUDIO:
        for (size_t i = 0; i < observers.size(); i++)
            this->observers.at(i)->onAudioBufferStateChanged(fillstateInPercent);
        break;
    case libdash::framework::buffer::VIDEO:
        for (size_t i = 0; i < observers.size(); i++)
            this->observers.at(i)->onVideoBufferStateChanged(fillstateInPercent);
    default:
        break;
    }
}

void MultimediaStream::setEOS(bool value)
{
    for(size_t i = 0; i < observers.size(); i++)
        this->observers.at(i)->setEOS(value);
}

void MultimediaStream::setTargetDownloadingTime(double target)
{
    this->dashManager->setTargetDownloadingTime(target);
}

void MultimediaStream::notifyStatistics(int segNum, uint32_t bitrate, int fps, uint32_t quality)
{
    for(size_t i = 0; i < observers.size(); i++)
        this->observers.at(i)->notifyStatistics(segNum, bitrate, fps, quality);
}

void	    MultimediaStream::notifyQualityDownloading(uint32_t quality)
{
    for(size_t i = 0; i < observers.size(); i++)
        this->observers.at(i)->notifyQualityDownloading(quality);
}

int	    MultimediaStream::getBufferLevel()
{
    int bufferFill = 0;
    for(size_t i=0; i < observers.size(); i++)
    {
        bufferFill = this->observers.at(i)->getBufferLevel();
    }
    return bufferFill;
}
bool MultimediaStream::canPush()
{
    bool flag = false;
    for(size_t i=0; i < observers.size(); i++)
    {
        flag = flag || this->observers.at(i)->canPush();
    }
    return flag;
}


libdash::framework::input::MediaObject* MultimediaStream::getSegment()
{
        return this->dashManager->getSegment();
}

void MultimediaStream::notifyBufferChange(uint32_t bufferfill, int maxC)
{
    this->dashManager->onBufferStateChanged(libdash::framework::buffer::VIDEO, bufferfill, maxC);
}

void MultimediaStream::updateMPD (IMPD* mpd)
{
//    this->mpd = mpd;
//    this->dashManager->updateMPD(mpd);
}

void MultimediaStream::fetchMPD()
{
    for(size_t i=0; i < this->observers.size(); i++)
    {
	this->observers.at(i)->fetchMPD();
    }
}
