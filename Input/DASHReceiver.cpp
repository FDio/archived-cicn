/*
 * DASHReceiver.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "DASHReceiver.h"
#include<stdio.h>

using namespace libdash::framework::input;
using namespace libdash::framework::buffer;
using namespace libdash::framework::mpd;
using namespace dash::mpd;

using duration_in_seconds = std::chrono::duration<double, std::ratio<1, 1> >;

DASHReceiver::DASHReceiver          (IMPD *mpd, IDASHReceiverObserver *obs, Buffer<MediaObject> *buffer, uint32_t bufferSize, bool icnEnabled, double icnAlpha, float beta, float drop) :
    mpd				(mpd),
    period			(NULL),
    adaptationSet		(NULL),
    representation		(NULL),
    adaptationSetStream		(NULL),
    representationStream	(NULL),
    segmentNumber       	(0),
    observer            	(obs),
    buffer              	(buffer),
    bufferSize          	(bufferSize),
    isBuffering         	(false),
    withFeedBack		(false),
    icn			(icnEnabled),
    icnAlpha			(icnAlpha),
    previousQuality		(0),
    isPaused			(false),
    threadComplete		(false),
    isScheduledPaced		(false),
    targetDownload  		(0.0),
    downloadingTime		(0.0),
    bufferLevelAtUpdate		(0),
    isBufferBased		(false),
    isLooping			(false),
    beta			(beta),
    drop			(drop)
{
    readMax = 32768;
    readBuffer = (uint8_t*)malloc(sizeof(uint8_t)*readMax);
    this->period = this->mpd->GetPeriods().at(0);
    this->adaptationSet = this->period->GetAdaptationSets().at(0);
    this->representation = this->adaptationSet->GetRepresentation().at(0);

    this->adaptationSetStream = new AdaptationSetStream(mpd, period, adaptationSet);
    this->representationStream = adaptationSetStream->getRepresentationStream(this->representation);
    this->segmentOffset = CalculateSegmentOffset();

    this->conn = NULL;
    this->initConn = NULL;
    readMax = 32768;
    readBuffer = (uint8_t *)malloc(sizeof(uint8_t) * readMax);

    if(icn)
    {
        this->conn = new ICNConnectionConsumerApi(this->icnAlpha, this->beta, this->drop);
        this->initConn = new ICNConnectionConsumerApi(this->icnAlpha, this->beta, this->drop);
    }
    InitializeCriticalSection(&this->monitorMutex);
    InitializeCriticalSection(&this->monitorPausedMutex);
    InitializeConditionVariable(&this->paused);
}
DASHReceiver::~DASHReceiver		()
{
    free(readBuffer);
    if(this->initConn)
        delete(this->initConn);
    if(this->conn)
        delete(this->conn);
    delete(this->adaptationSetStream);
    DeleteCriticalSection(&this->monitorMutex);
    DeleteCriticalSection(&this->monitorPausedMutex);
    DeleteConditionVariable(&this->paused);
}

void			DASHReceiver::SetAdaptationLogic(adaptation::IAdaptationLogic *_adaptationLogic)
{
    this->adaptationLogic = _adaptationLogic;
    this->isBufferBased = this->adaptationLogic->isBufferBased();
    this->withFeedBack = this->adaptationLogic->isRateBased();
}
bool			DASHReceiver::Start						()
{
    if(this->isBuffering)
        return false;

    this->isBuffering = true;
    this->bufferingThread = createThreadPortable(DoBuffering, this);

    if(this->bufferingThread == NULL)
    {
        this->isBuffering = false;
        return false;
    }

    return true;
}
void DASHReceiver::Stop()
{
    if(!this->isBuffering)
        return;

    this->isBuffering = false;
    this->buffer->setEOS(true);

    if(this->bufferingThread != NULL)
    {
        JoinThread(this->bufferingThread);
        destroyThreadPortable(this->bufferingThread);
    }
    this->period                = this->mpd->GetPeriods().at(0);
    this->adaptationSet         = this->period->GetAdaptationSets().at(0);
    this->representation        = this->adaptationSet->GetRepresentation().at(0);
}

MediaObject*	DASHReceiver::GetNextSegment	()
{
    ISegment *seg = NULL;

    EnterCriticalSection(&this->monitorPausedMutex);
    while(this->isPaused)
        SleepConditionVariableCS(&this->paused, &this->monitorPausedMutex, INFINITE);

    if(this->segmentNumber >= this->representationStream->getSize())
    {
        qDebug("looping? : %s\n", this->isLooping ? "YES" : "NO");
        if(this->isLooping)
        {
            this->segmentNumber = 0;
        }
        else
        {
            LeaveCriticalSection(&this->monitorPausedMutex);
            return NULL;
        }
    }
    seg = this->representationStream->getMediaSegment(this->segmentNumber + this->segmentOffset);

    if (seg != NULL)
    {
        std::vector<IRepresentation *> rep = this->adaptationSet->GetRepresentation();

        this->NotifyQualityDownloading(this->representation->GetBandwidth());

        MediaObject *media = new MediaObject(seg, this->representation,this->withFeedBack);
        this->segmentNumber++;
        LeaveCriticalSection(&this->monitorPausedMutex);
        return media;
    }
    LeaveCriticalSection(&this->monitorPausedMutex);
    return NULL;
}
MediaObject*	DASHReceiver::GetSegment		(uint32_t segNum)
{
    ISegment *seg = NULL;

    if(segNum >= this->representationStream->getSize())
        return NULL;

    seg = this->representationStream->getMediaSegment(segNum + segmentOffset);

    if (seg != NULL)
    {
        MediaObject *media = new MediaObject(seg, this->representation);
        return media;
    }

    return NULL;
}
MediaObject*	DASHReceiver::GetInitSegment	()
{
    ISegment *seg = NULL;

    seg = this->representationStream->getInitializationSegment();

    if (seg != NULL)
    {
        MediaObject *media = new MediaObject(seg, this->representation);
        return media;
    }

    return NULL;
}
MediaObject*	DASHReceiver::FindInitSegment	(dash::mpd::IRepresentation *representation)
{
    if (!this->InitSegmentExists(representation))
        return NULL;

    return this->initSegments[representation];
}
uint32_t                    DASHReceiver::GetPosition               ()
{
    return this->segmentNumber;
}
void			    DASHReceiver::SetLooping		    (bool looping)
{
    this->isLooping = looping;
}
void                        DASHReceiver::SetPosition               (uint32_t segmentNumber)
{
    this->segmentNumber = segmentNumber;
}
void                        DASHReceiver::SetPositionInMsecs        (uint32_t milliSecs)
{
    this->positionInMsecs = milliSecs;
}

void			    DASHReceiver::NotifyQualityDownloading  (uint32_t quality)
{
    this->observer->notifyQualityDownloading(quality);
}

void                        DASHReceiver::SetRepresentation         (IPeriod *period, IAdaptationSet *adaptationSet, IRepresentation *representation)
{
    EnterCriticalSection(&this->monitorMutex);

    bool periodChanged = false;

    if (this->representation == representation)
    {
        LeaveCriticalSection(&this->monitorMutex);
        return;
    }

    this->representation = representation;

    if (this->adaptationSet != adaptationSet)
    {
        this->adaptationSet = adaptationSet;

        if (this->period != period)
        {
            this->period = period;
            periodChanged = true;
        }

        delete this->adaptationSetStream;
        this->adaptationSetStream = NULL;

        this->adaptationSetStream = new AdaptationSetStream(this->mpd, this->period, this->adaptationSet);
    }

    this->representationStream  = this->adaptationSetStream->getRepresentationStream(this->representation);
    this->DownloadInitSegment(this->representation);

    if (periodChanged)
    {
        this->segmentNumber = 0;
        this->CalculateSegmentOffset();
    }
    LeaveCriticalSection(&this->monitorMutex);
}

libdash::framework::adaptation::IAdaptationLogic* DASHReceiver::GetAdaptationLogic	()
{
    return this->adaptationLogic;
}
dash::mpd::IRepresentation* DASHReceiver::GetRepresentation         ()
{
    return this->representation;
}
uint32_t                    DASHReceiver::CalculateSegmentOffset    ()
{
    if (mpd->GetType() == "static")
        return 0;

    uint32_t firstSegNum = this->representationStream->getFirstSegmentNumber();
    uint32_t currSegNum  = this->representationStream->getCurrentSegmentNumber();
    uint32_t startSegNum = currSegNum - 2*bufferSize;

    return (startSegNum > firstSegNum) ? startSegNum : firstSegNum;
}
void                        DASHReceiver::NotifySegmentDownloaded   ()
{
    this->observer->onSegmentDownloaded();
}

void						DASHReceiver::NotifyBitrateChange(dash::mpd::IRepresentation *representation)
{
    if(this->representation != representation)
    {
        this->representation = representation;
        this->SetRepresentation(this->period,this->adaptationSet,this->representation);
    }
}
void                        DASHReceiver::DownloadInitSegment    (IRepresentation* rep)
{
    if (this->InitSegmentExists(rep))
        return;

    MediaObject *initSeg = NULL;
    initSeg = this->GetInitSegment();

    if (initSeg)
    {
        initSeg->StartDownload(this->initConn);
        this->initSegments[rep] = initSeg;
        initSeg->WaitFinished();
    }
}
bool                        DASHReceiver::InitSegmentExists      (IRepresentation* rep)
{
    if (this->initSegments.find(rep) != this->initSegments.end())
        return true;

    return false;
}

void					DASHReceiver::Notifybps					(uint64_t bps)
{
    if(this->adaptationLogic)
    {
        if(this->withFeedBack)
        {
            this->adaptationLogic->bitrateUpdate(bps, this->segmentNumber);
        }
    }
}
void                                    DASHReceiver::NotifyDLTime                              (double time)
{
    if(this->adaptationLogic)
    {
        if(this->withFeedBack)
        {
            this->adaptationLogic->dLTimeUpdate(time);
        }
    }
}

void					DASHReceiver::NotifyCheckedAdaptationLogic()
{
    this->adaptationLogic->checkedByDASHReceiver();
}
//Is only called when this->adaptationLogic->IsBufferBased
void 					DASHReceiver::OnSegmentBufferStateChanged(uint32_t fillstateInPercent, int maxC)
{
    this->adaptationLogic->bufferUpdate(this->observer->getBufferLevel(), maxC);
    this->bufferLevelAtUpdate = this->observer->getBufferLevel();
}
void					DASHReceiver::OnEOS(bool value)
{
    this->adaptationLogic->onEOS(value);
}

bool			    DASHReceiver::PushBack(MediaObject *mediaObject)
{
    MediaObject *init = this->FindInitSegment(mediaObject->GetRepresentation());
    mediaObject->AddInitSegment(init);
    //TODO the read should be in a function

    //Grab the infos for the analytics: bitrate, fps
    dash::mpd::IRepresentation* datRep = mediaObject->GetRepresentation();
    uint32_t bitrate = 0;
    int fps = 0;
    uint32_t quality = 0;
    bitrate = datRep->GetBandwidth();
    quality = datRep->GetHeight();
    fps = this->bufferLevelAtUpdate;
    this->observer->notifyStatistics((int)this->segmentNumber - 1, bitrate, fps, quality);

    return(this->buffer->pushBack(mediaObject));
}

/* Thread that does the buffering of segments */
void*                       DASHReceiver::DoBuffering               (void *receiver)
{
    DASHReceiver *dashReceiver = (DASHReceiver *) receiver;

    dashReceiver->DownloadInitSegment(dashReceiver->GetRepresentation());

    MediaObject *media = dashReceiver->GetNextSegment();
    dashReceiver->NotifyCheckedAdaptationLogic();
    media->SetDASHReceiver(dashReceiver);
    std::chrono::time_point<std::chrono::system_clock> m_start_time = std::chrono::system_clock::now();
    while(media != NULL && dashReceiver->isBuffering)
    {
        //this is the case in PANDA
        if(dashReceiver->isScheduledPaced)
        {
            double delay = std::chrono::duration_cast<duration_in_seconds>(std::chrono::system_clock::now() - m_start_time).count();
            Debug("delay: %f, target: %f\n", delay, dashReceiver->targetDownload);
            if(delay < dashReceiver->targetDownload)
            {
                sleep(dashReceiver->targetDownload - delay);
            }
        }
        m_start_time = std::chrono::system_clock::now();
        media->StartDownload(dashReceiver->conn);

        media->WaitFinished();
        bool canPush = dashReceiver->CanPush();
        if (canPush && !dashReceiver->PushBack(media))
        {
            if(media)
            {
                delete(media);
            }
            media = NULL;
            dashReceiver->threadComplete = true;
            return NULL;
        }

        dashReceiver->NotifySegmentDownloaded();
        media = dashReceiver->GetNextSegment();
        dashReceiver->NotifyCheckedAdaptationLogic();
        if(media)
            media->SetDASHReceiver(dashReceiver);
    }

    dashReceiver->buffer->setEOS(true);
    dashReceiver->threadComplete = true;
    return NULL;
}

//can Push video to buffer in the renderer
bool					DASHReceiver::CanPush					()
{
    return this->observer->canPush();
}
void					DASHReceiver::ShouldAbort				()
{
    Debug("DASH RECEIVER SEGMENT --\n");
    this->segmentNumber--;
    Debug("DASH RECEIVER ABORT REQUEST\n");
}

void					DASHReceiver::SetTargetDownloadingTime	(double target)
{
    this->isScheduledPaced = true;
    this->targetDownload = target;
}

void					DASHReceiver::SetBeta	(float beta)
{
    this->beta = beta;
}

void					DASHReceiver::SetDrop	(float drop)
{
    this->drop = drop;
}


