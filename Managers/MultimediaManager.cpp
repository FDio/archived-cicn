/*
 * MultimediaManager.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "MultimediaManager.h"

#include <fstream>

using namespace libdash::framework::adaptation;
using namespace libdash::framework::buffer;
using namespace viper::managers;
using namespace dash::mpd;

#include <queue>

MultimediaManager::MultimediaManager(ViperGui *viperGui, int segBufSize, std::string downloadPath, bool nodecoding) :
    viperGui                    (viperGui),
    segmentBufferSize           (segBufSize),
    downloadPath                (downloadPath),
    offset                      (offset),
    mpd                         (NULL),
    period                      (NULL),
    videoAdaptationSet          (NULL),
    videoRepresentation         (NULL),
    videoLogic                  (NULL),
    videoStream                 (NULL),
    audioAdaptationSet          (NULL),
    audioRepresentation         (NULL),
    audioLogic                  (NULL),
    videoRendererHandle	 	    (NULL),
    audioRendererHandle	 	    (NULL),
    audioStream                 (NULL),
    started                     (false),
    stopping                    (false),
    framesDisplayed             (0),
    segmentsDownloaded          (0),
    isVideoRendering            (false),
    isAudioRendering            (false),
    eos                         (false),
    playing                     (false),
    noDecoding                  (nodecoding)
{
    InitializeCriticalSection (&this->monitorMutex);
    InitializeCriticalSection (&this->monitorBufferMutex);
    InitializeCriticalSection (&this->monitor_playing_video_mutex);
    InitializeConditionVariable (&this->playingVideoStatusChanged);
    InitializeCriticalSection (&this->monitor_playing_audio_mutex);
    InitializeConditionVariable (&this->playingAudioStatusChanged);

    this->manager = CreateDashManager();
}

MultimediaManager::~MultimediaManager           ()
{
    this->stop();
    this->manager->Delete();
    DeleteCriticalSection (&this->monitorMutex);
    DeleteCriticalSection (&this->monitorBufferMutex);
    DeleteCriticalSection (&this->monitor_playing_video_mutex);
    DeleteConditionVariable (&this->playingVideoStatusChanged);
    DeleteCriticalSection (&this->monitor_playing_audio_mutex);
    DeleteConditionVariable (&this->playingAudioStatusChanged);
}

IMPD* MultimediaManager::getMPD()
{
    return this->mpd;
}

bool    MultimediaManager::init(const std::string& url)
{
    EnterCriticalSection(&this->monitorMutex);
    this->mpd = this->manager->Open((char *)url.c_str());
    Debug("url : %s\n", url.c_str());
    if(this->mpd == NULL)
    {
        LeaveCriticalSection(&this->monitorMutex);
        return false;
    }
    Debug("Done DL the mpd\n");
    LeaveCriticalSection(&this->monitorMutex);
    return true;
}

bool    MultimediaManager::initICN(const std::string& url)
{
    EnterCriticalSection(&this->monitorMutex);
    libdash::framework::input::IICNConnection* icnConn = new libdash::framework::input::ICNConnectionConsumerApi(20.0, this->beta, this->drop);
    icnConn->InitForMPD(url);
    int ret = 0;
    char * data = (char *)malloc(4096);
    int pos = url.find_last_of("/");
    if(pos == std::string::npos)
    {
        pos = strlen(url.c_str());
    }
    else
    {
        pos = pos + 1;
    }

    std::string downloadFile(this->downloadPath + url.substr(pos).c_str());
    FILE *fp;
    fp = fopen(downloadFile.c_str(), "w");
    if(fp == NULL)
    {
        free(data);
        delete icnConn;
        LeaveCriticalSection(&this->monitorMutex);
        return false;
    }
    ret = icnConn->Read((uint8_t*)data, 4096);
    while(ret)
    {
        fwrite(data, sizeof(char), ret, fp);
        ret = icnConn->Read((uint8_t*)data,4096);
    }
    fclose(fp);
    this->mpd = this->manager->Open(const_cast<char*>(downloadFile.c_str()), url);
    if(this->mpd == NULL)
    {
        remove(downloadFile.c_str());
        free(data);
        delete icnConn;
        LeaveCriticalSection(&this->monitorMutex);
        return false;
    }
    remove(downloadFile.c_str());
    free(data);
    delete icnConn;
    LeaveCriticalSection(&this->monitorMutex);
    return true;
}

bool MultimediaManager::isStarted()
{
    return this->started;
}

bool MultimediaManager::isStopping()
{
    return this->stopping;
}

bool MultimediaManager::isICN()
{
    return this->icn;
}

void MultimediaManager::start(bool icnEnabled, double icnAlpha, uint32_t nextOffset)
{
    this->icn = icnEnabled;
    this->icnAlpha = icnAlpha;
    if (this->started)
        this->stop();
    if(icnAlpha <= 1 && icnAlpha >=0)
    {
        qDebug("ICN-enhanced rate estimation: alpha = %f\n",icnAlpha);

    } else {
        qDebug("normal rate estimation\n");
    }
    EnterCriticalSection(&this->monitorMutex);
    if (this->videoAdaptationSet && this->videoRepresentation)
    {
        this->initVideoRendering(nextOffset);
        this->videoStream->setAdaptationLogic(this->videoLogic);
        this->videoLogic->setMultimediaManager(this);
        this->videoStream->start();
        this->startVideoRenderingThread();
    }
    this->started = true;
    this->playing = true;
    LeaveCriticalSection(&this->monitorMutex);
}

void MultimediaManager::stop()
{
    if (!this->started)
        return;
    this->stopping = true;
    EnterCriticalSection(&this->monitorMutex);
    this->stopVideo();
    this->stopping = false;
    this->started = false;
    LeaveCriticalSection(&this->monitorMutex);
    Debug("VIDEO STOPPED\n");
    this->period                = this->mpd->GetPeriods().at(0);
    this->videoAdaptationSet    = this->period->GetAdaptationSets().at(0);
    this->videoRepresentation   = this->videoAdaptationSet->GetRepresentation().at(0);

}

void MultimediaManager::stopVideo()
{
    if(this->started && this->videoStream)
    {
        this->videoStream->stop();
        this->stopVideoRenderingThread();
        delete this->videoStream;
        delete this->videoLogic;
        this->videoStream = NULL;
        this->videoLogic = NULL;
    }
}

void MultimediaManager::stopAudio()
{
    if (this->started && this->audioStream)
    {
        //TODO add audio support
    }
}

bool MultimediaManager::setVideoQuality(IPeriod* period, IAdaptationSet *adaptationSet, IRepresentation *representation)
{
    EnterCriticalSection(&this->monitorMutex);

    this->period                = period;
    this->videoAdaptationSet    = adaptationSet;
    this->videoRepresentation   = representation;
    if (this->videoStream)
        this->videoStream->setRepresentation(this->period, this->videoAdaptationSet, this->videoRepresentation);

    LeaveCriticalSection(&this->monitorMutex);
    return true;
}

bool MultimediaManager::setAudioQuality(IPeriod* period, IAdaptationSet *adaptationSet, IRepresentation *representation)
{
    EnterCriticalSection(&this->monitorMutex);

    this->period                = period;
    this->audioAdaptationSet    = adaptationSet;
    this->audioRepresentation   = representation;
    if (this->audioStream)
        this->audioStream->setRepresentation(this->period, this->audioAdaptationSet, this->audioRepresentation);
    LeaveCriticalSection(&this->monitorMutex);
    return true;
}

bool MultimediaManager::isUserDependent()
{
    if(this->videoLogic)
        return this->videoLogic->isUserDependent();
    else
        return true;
}

bool MultimediaManager::setVideoAdaptationLogic(libdash::framework::adaptation::LogicType type, struct libdash::framework::adaptation::AdaptationParameters *params)
{
    if(this->videoAdaptationSet)
    {
        if(this->videoLogic)
            delete(this->videoLogic);
        this->videoLogic = AdaptationLogicFactory::create(type, this->mpd, this->period, this->videoAdaptationSet, 1, params);
        this->logicName = LogicType_string[type];
    }
    else
    {
        this->videoLogic = NULL;
        return true;
    }
    if(this->videoLogic)
        return true;
    else
        return false;
}

void MultimediaManager::shouldAbort(bool isVideo)
{
    if(isVideo)
    {
        this->videoStream->shouldAbort();
        return;
    }
    else
    {
        this->audioStream->shouldAbort();
    }
}

void MultimediaManager::setTargetDownloadingTime(bool isVideo, double target)
{
    if(isVideo)
        this->videoStream->setTargetDownloadingTime(target);
    else
        this->audioStream->setTargetDownloadingTime(target);
}

bool MultimediaManager::setAudioAdaptationLogic(libdash::framework::adaptation::LogicType type, struct libdash::framework::adaptation::AdaptationParameters *params)
{
    if(this->audioAdaptationSet)
    {
        this->audioLogic = AdaptationLogicFactory::create(type, this->mpd, this->period, this->audioAdaptationSet, 0, params);
        this->logicName = LogicType_string[type];
    }
    else
    {
        this->audioLogic = NULL;
        return true;
    }
    if(this->audioLogic)
        return true;
    else
        return false;
}

void MultimediaManager::attachManagerObserver(IMultimediaManagerObserver *observer)
{
    this->managerObservers.push_back(observer);
}

void MultimediaManager::notifyVideoBufferObservers(uint32_t fillstateInPercent)
{
    for (size_t i = 0; i < this->managerObservers.size(); i++)
        this->managerObservers.at(i)->onVideoBufferStateChanged(fillstateInPercent);
}

void MultimediaManager::notifyVideoSegmentBufferObservers(uint32_t fillstateInPercent)
{
    for (size_t i = 0; i < this->managerObservers.size(); i++)
        this->managerObservers.at(i)->onVideoSegmentBufferStateChanged(fillstateInPercent);
}

void MultimediaManager::notifyAudioSegmentBufferObservers(uint32_t fillstateInPercent)
{
    for (size_t i = 0; i < this->managerObservers.size(); i++)
        this->managerObservers.at(i)->onAudioSegmentBufferStateChanged(fillstateInPercent);
}

void MultimediaManager::notifyAudioBufferObservers(uint32_t fillstateInPercent)
{
    for (size_t i = 0; i < this->managerObservers.size(); i++)
        this->managerObservers.at(i)->onAudioBufferStateChanged(fillstateInPercent);
}

void MultimediaManager::initVideoRendering(uint32_t offset)
{
    this->videoStream = new MultimediaStream(viper::managers::VIDEO, this->mpd, this->segmentBufferSize, this->isICN(), this->icnAlpha, this->noDecoding, this->beta, this->drop);
    this->videoStream->attachStreamObserver(this);
    this->videoStream->setRepresentation(this->period, this->videoAdaptationSet, this->videoRepresentation);
    this->videoStream->setPosition(offset);
}

void MultimediaManager::initAudioPlayback(uint32_t offset)
{
    this->audioStream = new MultimediaStream(viper::managers::AUDIO, this->mpd, this->segmentBufferSize, this->isICN(), this->icnAlpha, this->noDecoding, this->beta, this->drop);
    this->audioStream->attachStreamObserver(this);
    this->audioStream->setRepresentation(this->period, this->audioAdaptationSet, this->audioRepresentation);
    this->audioStream->setPosition(offset);
}

void MultimediaManager::setLooping(bool looping)
{
    if(this->videoStream)
    {
        this->videoStream->setLooping(looping);
    }
    if(this->audioStream)
    {
        this->audioStream->setLooping(looping);
    }
}

void    MultimediaManager::onSegmentDownloaded              ()
{
    this->segmentsDownloaded++;
}

void MultimediaManager::onSegmentBufferStateChanged(StreamType type, uint32_t fillstateInPercent, int maxC)
{
    switch (type)
    {
    case AUDIO:
        this->notifyAudioSegmentBufferObservers(fillstateInPercent);
        break;
    case VIDEO:
        this->notifyVideoSegmentBufferObservers(fillstateInPercent);
        break;
    default:
        break;
    }
}

void MultimediaManager::onVideoBufferStateChanged(uint32_t fillstateInPercent)
{
    this->notifyVideoBufferObservers(fillstateInPercent);
}

void MultimediaManager::onAudioBufferStateChanged(uint32_t fillstateInPercent)
{
    this->notifyAudioBufferObservers(fillstateInPercent);
}

void MultimediaManager::setFrameRate(double framerate)
{
    this->frameRate = framerate;
}

void MultimediaManager::setEOS(bool value)
{
    this->eos = value;
    if(value) //ie: End of Stream so the rendering thread(s) will finish
    {
        this->stopping = true;
        if(this->videoRendererHandle != NULL)
        {
            this->stopVideoRenderingThread();
            this->videoRendererHandle = NULL;
        }
        if(this->audioRendererHandle != NULL)
        {
            this->stopAudioRenderingThread();
            this->audioRendererHandle = NULL;
        }
        this->stopping = false;
        for(size_t i = 0; i < this->managerObservers.size(); i++)
            this->managerObservers.at(i)->onEOS();
    }
}

bool MultimediaManager::startVideoRenderingThread()
{
    this->isVideoRendering = true;
    if(!noDecoding)
        this->videoRendererHandle = createThreadPortable (pushVideo, this);
    else
        this->videoRendererHandle = createThreadPortable (pushVideoNoOut, this);

    if(this->videoRendererHandle == NULL)
        return false;

    return true;
}

void MultimediaManager::stopVideoRenderingThread()
{
    this->isVideoRendering = false;
    if (this->videoRendererHandle != NULL)
    {
        JoinThread(this->videoRendererHandle);
        destroyThreadPortable(this->videoRendererHandle);
    }
}

bool MultimediaManager::startAudioRenderingThread()
{
    this->isAudioRendering = true;
    if(this->audioRendererHandle == NULL)
        return false;
    return true;
}

void MultimediaManager::stopAudioRenderingThread()
{
    this->isAudioRendering = false;
    if (this->audioRendererHandle != NULL)
    {
        JoinThread(this->audioRendererHandle);
        destroyThreadPortable(this->audioRendererHandle);
    }
}

bool MultimediaManager::isPlaying()
{
    return this->playing;
}

void MultimediaManager::onPausePressed()
{
    EnterCriticalSection(&this->monitor_playing_video_mutex);
    EnterCriticalSection(&this->monitor_playing_audio_mutex);
    this->playing = !this->playing;
    WakeAllConditionVariable(&this->playingVideoStatusChanged);
    WakeAllConditionVariable(&this->playingAudioStatusChanged);
    LeaveCriticalSection(&this->monitor_playing_video_mutex);
    LeaveCriticalSection(&this->monitor_playing_audio_mutex);
}

void* MultimediaManager::pushVideoNoOut(void *data)
{
    MultimediaManager *manager = (MultimediaManager*) data;
    manager->lastPointInTime = std::chrono::system_clock::now();
    manager->bufferingLimit = manager->lastPointInTime;
    libdash::framework::input::MediaObject *segment = manager->videoStream->getSegment();
    using duration_in_milliSeconds = std::chrono::duration<long int, std::ratio<1,1000>>;
    std::chrono::time_point<std::chrono::system_clock> timeOfInsertion;
    std::chrono::time_point<std::chrono::system_clock> startTime;
    long int actualPosition;

    while(manager->isVideoRendering)
    {
        if (segment)
        {
            timeOfInsertion = std::chrono::system_clock::now();
            actualPosition = std::chrono::duration_cast<duration_in_milliSeconds>(manager->bufferingLimit - timeOfInsertion).count();
            if(actualPosition < 0)
            {
                Debug("MANAGER:\tRebuffered %d ms\n", actualPosition *(-1));
                manager->lastPointInTime = timeOfInsertion;
                //TODO Replace the 2 by a variable with segmentDuration
                manager->bufferingLimit = manager->lastPointInTime + std::chrono::seconds(2);
            }
            else
            {
                //TODO Replace the 2 by a variable with segmentDuration
                Debug("MANAGER: INSERT TO BUFFER old_fillness: %f, new_fillness: %f\n", (double)((double)actualPosition/1000.0) / (double) this->segmentBufferSize, (double)((double)(actualPosition + 2000)/1000.0) / (double) manager->segmentBufferSize);
                manager->bufferingLimit = manager->bufferingLimit + std::chrono::seconds(2);
                manager->lastPointInTime = timeOfInsertion;
            }
            delete segment;
        }
        else
        {
            //noDecoding here means noGUI
            if(manager->noDecoding)
                manager->setEOS(true);
        }
        segment = manager->videoStream->getSegment();
    }
    return NULL;

}

void MultimediaManager::notifyStatistics(int segNum, uint32_t bitrate, int fps, uint32_t quality)
{
    for(size_t i = 0; i < this->managerObservers.size(); i++)
    {
        this->managerObservers.at(i)->notifyStatistics(segNum, bitrate, fps, quality);
    }
}

void MultimediaManager::notifyQualityDownloading(uint32_t quality)
{
    for(size_t i = 0; i < this->managerObservers.size(); i++)
    {
        this->managerObservers.at(i)->notifyQualityDownloading(quality);
    }
}

void MultimediaManager::notifyBufferChange()
{
    if(this->videoStream)
    {
        this->videoStream->notifyBufferChange(this->getUBufferLevel(), this->segmentBufferSize);
    }
    if(this->audioStream)
    {
        this->audioStream->notifyBufferChange(this->getUBufferLevel(), this->segmentBufferSize);
    }
}

int MultimediaManager::getBufferLevel()
{
    return (int)this->getUBufferLevel();
}

uint32_t MultimediaManager::getUBufferLevel()
{
    int mBufferLevel = 0;
    int segmentDurationInMs = 2000;

    if(noDecoding)
    {
        std::chrono::time_point<std::chrono::system_clock> timeNow = std::chrono::system_clock::now();
        using duration_in_milliSeconds = std::chrono::duration<long int, std::ratio<1,1000>>;
        long int actualPos = std::chrono::duration_cast<duration_in_milliSeconds>(this->bufferingLimit - timeNow).count();
        int res = ((double)actualPos) / (double (this->segmentBufferSize * segmentDurationInMs)) * 100;
        return res >= 0 ? res > 100 ? 100 : (uint32_t) res : 0;
    }
    else
    {
        mBufferLevel = this->viperGui->getBufferDuration();
        int res = ((int)mBufferLevel)/((double) (this->segmentBufferSize * this->viperGui->getSegmentDuration())) * 100;
        return res >= 0 ? res > 100 ? 100 : (uint32_t) res : 0;

    }
}

bool MultimediaManager::canPush()
{
    int segmentDurationInMs = 2000;
    while(this->getUBufferLevel() >= 100 && !this->stopping)
    {
        sleep(segmentDurationInMs / 1000);
    }
    return true;
}

void* MultimediaManager::pushVideo(void *data)
{
    MultimediaManager *manager = (MultimediaManager*) data;
    libdash::framework::input::MediaObject *segment = manager->videoStream->getSegment();
    long int segmentDurationInMs = 2000;
    while(manager->isVideoRendering)
    {
        if (segment)
        {
            manager->notifyBufferChange();
            manager->viperGui->writeData(segment);
            delete segment;
        }
        segment = manager->videoStream->getSegment();
    }
    return NULL;
}

void MultimediaManager::setOffset(int offset)
{
    this->offset = offset;
}

void MultimediaManager::setBeta(float beta)
{
    this->beta = beta;
}

void MultimediaManager::setDrop(float drop)
{
    this->drop = drop;
}
