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
using namespace libdash::framework::mpd;
using namespace libdash::framework::buffer;
using namespace viper::managers;
using namespace dash::mpd;

#include <queue>

MultimediaManager::MultimediaManager(ViperGui *viperGui, int segBufSize, std::string downloadPath, bool nodecoding) :
    viperGui                    (viperGui),
    segmentBufferSize           (segBufSize),
    downloadPath                (downloadPath),
    offset                      (offset),
    videoLogic                  (NULL),
    videoStream                 (NULL),
    audioLogic                  (NULL),
    videoRendererHandle         (NULL),
    audioRendererHandle         (NULL),
    audioStream                 (NULL),
    started                     (false),
    stopping                    (false),
    framesDisplayed             (0),
    segmentsDownloaded          (0),
    isVideoRendering            (false),
    isAudioRendering            (false),
    eos                         (false),
    playing                     (false),
    noDecoding                  (nodecoding),
    mpdWrapper                  (NULL),
    mpdFetcherThread            (NULL)
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
    return this->mpdWrapper->getMPD();
}

MPDWrapper*	MultimediaManager::getMPDWrapper()
{
    return this->mpdWrapper;
}

bool    MultimediaManager::init(const std::string& url)
{
    this->url = url;
    EnterCriticalSection(&this->monitorMutex);
    IMPD* mpd = this->manager->Open((char *)url.c_str());
    Debug("url : %s\n", url.c_str());
    if(mpd == NULL)
    {
        LeaveCriticalSection(&this->monitorMutex);
        return false;
    }
    Debug("Done DL the mpd\n");
    this->mpdWrapper->setIsStopping(false);
    this->mpdWrapper->updateMPD(mpd);
    for (size_t i = 0; i < this->managerObservers.size(); i++)
        this->managerObservers.at(i)->setMPDWrapper(this->mpdWrapper);
    LeaveCriticalSection(&this->monitorMutex);
    return true;
}

bool    MultimediaManager::initICN(const std::string& url)
{
    this->url = url;
    EnterCriticalSection(&this->monitorMutex);
    this->icnConn = new libdash::framework::input::ICNConnectionConsumerApi(20.0, this->beta, this->drop);
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
    IMPD* mpd = this->manager->Open(const_cast<char*>(downloadFile.c_str()), url);
    remove(downloadFile.c_str());
    free(data);
    if(mpd == NULL)
    {
        delete icnConn;
        LeaveCriticalSection(&this->monitorMutex);
        return false;
    }
    this->mpdWrapper->setIsStopping(false);
    this->mpdWrapper->updateMPD(mpd);
    for (size_t i = 0; i < this->managerObservers.size(); i++)
        this->managerObservers.at(i)->setMPDWrapper(this->mpdWrapper);
    if( !strcmp(this->mpdWrapper->getType().c_str(), "static") )
    {
        delete icnConn;
    }

    LeaveCriticalSection(&this->monitorMutex);
    return true;
}

void MultimediaManager::updateMPD()
{
    this->mpdWrapper->updateMPD(this->manager->Open((char *)url.c_str()));
}

void MultimediaManager::updateMPDICN()
{
    this->icnConn->InitForMPD(this->url);
    int ret = 0;
    char * data = (char *)malloc(4096);
    int pos = this->url.find_last_of("/");
    if(pos == std::string::npos)
    {
        pos = strlen(this->url.c_str());
    }
    else
    {
        pos = pos + 1;
    }

    std::string downloadFile(this->downloadPath + this->url.substr(pos).c_str());
    FILE *fp;
    fp = fopen(downloadFile.c_str(), "w");
    if(fp == NULL)
    {
        free(data);
	return;
    }
    ret = icnConn->Read((uint8_t*)data, 4096);
    while(ret)
    {
        fwrite(data, sizeof(char), ret, fp);
        ret = icnConn->Read((uint8_t*)data,4096);
    }
    fclose(fp);
    this->mpdWrapper->updateMPD(this->manager->Open(const_cast<char*>(downloadFile.c_str()), this->url));

    remove(downloadFile.c_str());
    free(data);
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

void MultimediaManager::setMPDWrapper(MPDWrapper* mpdWrapper)
{
    this->mpdWrapper = mpdWrapper;
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
    if(this->mpdWrapper->hasVideoAdaptationSetAndVideoRepresentation())
    {
        this->initVideoRendering(nextOffset);
        this->videoStream->setAdaptationLogic(this->videoLogic);
        this->videoLogic->setMultimediaManager(this);
        this->videoStream->start();
        this->startVideoRenderingThread();
    }
    this->started = true;
    this->playing = true;

    if(!strcmp(this->mpdWrapper->getType().c_str(), "dynamic"))
    {
	    this->mpdFetcherThread = createThreadPortable(DoMPDFetching, this);
	    if(this->mpdFetcherThread == NULL)
	    {
	        std::cout << "mpd Fetcher thread is NULL. Need to think of how to handle this?" << std::endl;
	    }
	}
    LeaveCriticalSection(&this->monitorMutex);
}

void MultimediaManager::stop()
{
    if (!this->started)
        return;
        
    this->mpdWrapper->setIsStopping(true);
    this->stopping = true;
    EnterCriticalSection(&this->monitorMutex);
    this->stopVideo();
    this->stopping = false;
    this->started = false;
    LeaveCriticalSection(&this->monitorMutex);
    Debug("VIDEO STOPPED\n");
    this->mpdWrapper->reInit(viper::managers::StreamType::VIDEO);
    this->mpdWrapper->reInit(viper::managers::StreamType::AUDIO);
    if(this->mpdFetcherThread != NULL)
    {
	JoinThread(this->mpdFetcherThread);
	destroyThreadPortable(this->mpdFetcherThread);
    }
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

bool MultimediaManager::setVideoQuality()
{

    if (this->videoStream)
        this->videoStream->setRepresentation();
    return true;
}

bool MultimediaManager::setAudioQuality()
{
    if (this->audioStream)
        this->audioStream->setRepresentation();
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
    if(this->mpdWrapper->hasVideoAdaptationSetAndVideoRepresentation())
    {
        if(this->videoLogic)
            delete(this->videoLogic);
        this->videoLogic = AdaptationLogicFactory::create(type, viper::managers::StreamType::VIDEO, this->mpdWrapper, params);
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
    if(this->mpdWrapper->hasAudioAdaptationSetAndAudioRepresentation())
    {
        if(this->audioLogic)
            delete(this->audioLogic);
        this->audioLogic = AdaptationLogicFactory::create(type, viper::managers::StreamType::AUDIO, this->mpdWrapper, params);
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
    this->videoStream = new MultimediaStream(viper::managers::VIDEO, this->mpdWrapper, this->segmentBufferSize, this->isICN(), this->icnAlpha, this->noDecoding, this->beta, this->drop);
    this->videoStream->attachStreamObserver(this);
    this->videoStream->setPosition(offset);
}

void MultimediaManager::initAudioPlayback(uint32_t offset)
{
    this->audioStream = new MultimediaStream(viper::managers::AUDIO, this->mpdWrapper, this->segmentBufferSize, this->isICN(), this->icnAlpha, this->noDecoding, this->beta, this->drop);
    this->audioStream->attachStreamObserver(this);
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
                manager->bufferingLimit = manager->lastPointInTime + std::chrono::seconds(((int)manager->getSegmentDuration() / 1000));
            }
            else
            {
                Debug("MANAGER: INSERT TO BUFFER old_fillness: %f, new_fillness: %f\n", (double)((double)actualPosition/1000.0) / (double) this->segmentBufferSize, (double)((double)(actualPosition + 2000)/1000.0) / (double) manager->segmentBufferSize);
                manager->bufferingLimit = manager->bufferingLimit + std::chrono::seconds(((int)manager->getSegmentDuration() /1000));
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
    int segmentDurationInMs = (int) this->segmentDuration;

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
    int segmentDurationInMs = (int)this->segmentDuration;
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

void MultimediaManager::fetchMPD()
{
    if(this->icn)
	this->updateMPDICN();
    else
	this->updateMPD();
}

//SegmentDuration is in ms
void MultimediaManager::setSegmentDuration(float segDuration)
{
    this->segmentDuration = segDuration;
}

float MultimediaManager::getSegmentDuration()
{
    return this->segmentDuration;
}

void*					MultimediaManager::DoMPDFetching				(void* data)
{
    MultimediaManager *manager = (MultimediaManager*) data;
    uint32_t currTime = TimeResolver::getCurrentTimeInSec();
    uint32_t publishedTime = manager->mpdWrapper->getFetchTime();
//    To avoid clock synchronisation issues: using fetching time instead of publish time
//    uint32_t publishedTime = TimeResolver::getUTCDateTimeInSec(dashReceiver->mpdWrapper->getPublishTime());
    uint32_t periodUpdate = TimeResolver::getDurationInSec(manager->mpdWrapper->getMinimumUpdatePeriod());
   while(manager->isStarted())
    {
    while(manager->isStarted() && currTime < publishedTime + periodUpdate)
    {
        usleep(((publishedTime + periodUpdate) - currTime) * 1000000);
        currTime = TimeResolver::getCurrentTimeInSec();
    }
    manager->fetchMPD();
	publishedTime = manager->mpdWrapper->getFetchTime();
//	publishedTime = TimeResolver::getUTCDateTimeInSec(dashReceiver->mpdWrapper->getPublishTime());
	periodUpdate = TimeResolver::getDurationInSec(manager->mpdWrapper->getMinimumUpdatePeriod());
    }
}
