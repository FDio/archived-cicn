/*
 * MediaObject.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "MediaObject.h"
#include <inttypes.h>
#include<stdio.h>

using namespace libdash::framework::input;
using namespace dash::mpd;
using namespace dash::network;
using namespace dash::metrics;

MediaObject::MediaObject(ISegment *segment, IRepresentation *rep, bool withFeedBack) :
    segment        (segment),
    rep            (rep),
    withFeedBack	(withFeedBack)
{
    this->initSeg = NULL;
    InitializeConditionVariable (&this->stateChanged);
    InitializeCriticalSection   (&this->stateLock);
}

MediaObject::~MediaObject()
{
    if(this->state == IN_PROGRESS)
    {
        this->segment->AbortDownload();
        this->OnDownloadStateChanged(ABORTED);
    }
    this->segment->DetachDownloadObserver(this);
    this->WaitFinished();

    DeleteConditionVariable (&this->stateChanged);
    DeleteCriticalSection   (&this->stateLock);
    delete this->segment;
    this->segment = NULL;
}

void MediaObject::SetFeedBack(bool flag)
{
    this->withFeedBack = flag;
}

void MediaObject::AddInitSegment(MediaObject* initSeg)
{
    this->initSeg = initSeg;
}

int MediaObject::ReadInitSegment(uint8_t* data, size_t len)
{
    if(this->initSeg)
        return this->initSeg->Peek(data,len);
    else
        return 0;
}

bool MediaObject::StartDownload()
{
    this->segment->AttachDownloadObserver(this);
    return this->segment->StartDownload();
}

bool MediaObject::StartDownload(IICNConnection* conn)
{
    if(conn == NULL)
        return this->StartDownload();

    conn->Init(this->segment);

    this->segment->AttachDownloadObserver(this);
    return this->segment->StartDownload(conn);
}

const char* MediaObject::GetPath()
{
    return ((IChunk*)this->segment)->Path().c_str();
}

void MediaObject::AbortDownload()
{
    this->segment->AbortDownload();
    this->OnDownloadStateChanged(ABORTED);
}

void MediaObject::WaitFinished()
{
    EnterCriticalSection(&this->stateLock);

    while(this->state != COMPLETED && this->state != ABORTED){
        SleepConditionVariableCS(&this->stateChanged, &this->stateLock, INFINITE);
    }

    LeaveCriticalSection(&this->stateLock);
    if(this->state != ABORTED)
    {
        if(this->withFeedBack && this->dashReceiver)
        {
            this->dashReceiver->Notifybps(this->bps);
            this->dashReceiver->NotifyDLTime(this->dnltime);
        }
    }
}

int MediaObject::Read(uint8_t *data, size_t len)
{
    return this->segment->Read(data, len);
}

int MediaObject::Peek(uint8_t *data, size_t len)
{
    return this->segment->Peek(data, len);
}

int MediaObject::Peek(uint8_t *data, size_t len, size_t offset)
{
    return this->segment->Peek(data, len, offset);
}

IRepresentation* MediaObject::GetRepresentation()
{
    return this->rep;
}

ISegment* MediaObject::GetSegment()
{
    return this->segment;
}

void MediaObject::OnDownloadTimeChanged(double dnltime)
{
    this->dnltime = dnltime;
}

void MediaObject::OnDownloadStateChanged(DownloadState state)
{
    EnterCriticalSection(&this->stateLock);

    this->state = state;

    WakeAllConditionVariable(&this->stateChanged);
    LeaveCriticalSection(&this->stateLock);
}

void MediaObject::OnDownloadRateChanged(uint64_t bitsPerSecond)
{
    this->bps = bitsPerSecond;
}

void MediaObject::SetDASHReceiver(input::DASHReceiver *_dashReceiver)
{
    this->dashReceiver = _dashReceiver;
}

void MediaObject::SetAdaptationLogic(adaptation::IAdaptationLogic *_adaptationLogic)
{
    this->adaptationLogic = _adaptationLogic;
}

const std::vector<ITCPConnection *>& MediaObject::GetTCPConnectionList() const
{
    return this->segment->GetTCPConnectionList();
}

const std::vector<IHTTPTransaction *>& MediaObject::GetHTTPTransactionList () const
{
    return this->segment->GetHTTPTransactionList();
}
