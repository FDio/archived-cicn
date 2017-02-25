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

#include "BufferBasedAdaptation.h"
#include<stdio.h>


using namespace dash::mpd;
using namespace libdash::framework::adaptation;
using namespace libdash::framework::input;
using namespace libdash::framework::mpd;

BufferBasedAdaptation::BufferBasedAdaptation          (IMPD *mpd, IPeriod *period, IAdaptationSet *adaptationSet, bool isVid, struct AdaptationParameters *params) :
    AbstractAdaptationLogic   (mpd, period, adaptationSet, isVid)
{
    this->reservoirThreshold = params->BufferBased_reservoirThreshold;
    this->maxThreshold = params->BufferBased_maxThreshold;

    std::vector<IRepresentation* > representations = this->adaptationSet->GetRepresentation();

    this->representation = this->adaptationSet->GetRepresentation().at(0);
    this->multimediaManager = NULL;
    this->lastBufferFill = 0;
    this->bufferEOS = false;
    this->shouldAbort = false;
    Debug("BufferBasedParams:\t%f\t%f\n", (double)reservoirThreshold/100, (double)maxThreshold/100);
    Debug("Buffer Adaptation:	STARTED\n");
}

BufferBasedAdaptation::~BufferBasedAdaptation         ()
{
}

LogicType BufferBasedAdaptation::getType()
{
    return adaptation::BufferBased;
}

bool BufferBasedAdaptation::isUserDependent()
{
    return false;
}

bool BufferBasedAdaptation::isRateBased()
{
    return false;
}
bool BufferBasedAdaptation::isBufferBased()
{
    return true;
}

void BufferBasedAdaptation::setMultimediaManager (viper::managers::IMultimediaManagerBase *_mmManager)
{
    this->multimediaManager = _mmManager;
}

void BufferBasedAdaptation::notifyBitrateChange()
{
    if(this->multimediaManager)
        if(this->multimediaManager->isStarted() && !this->multimediaManager->isStopping())
            if(this->isVideo)
                this->multimediaManager->setVideoQuality(this->period, this->adaptationSet, this->representation);
            else
                this->multimediaManager->setAudioQuality(this->period, this->adaptationSet, this->representation);

    if(this->shouldAbort)
    {
        this->multimediaManager->shouldAbort(this->isVideo);
    }
    this->shouldAbort = false;
}

uint64_t BufferBasedAdaptation::getBitrate()
{
    return this->currentBitrate;
}

void BufferBasedAdaptation::setBitrate(uint32_t bufferFill)
{
    std::vector<IRepresentation *> representations;
    representations = this->adaptationSet->GetRepresentation();
    size_t i = 0;

    if(representations.size() == 1)
    {
        i = 0;
    }
    else
    {
        while(bufferFill > this->reservoirThreshold + i * (this->maxThreshold - this->reservoirThreshold)/(representations.size()-1))
        {
            i++;
        }
    }
    if((size_t)i >= (size_t)(representations.size()))
        i = representations.size() - 1;
    this->representation = representations.at(i);
    if( 0 && !this->bufferEOS && this->lastBufferFill > this->reservoirThreshold && bufferFill <= this->reservoirThreshold)
    {
        this->shouldAbort = true;
    }
    Debug("ADAPTATION_LOGIC:\tFor %s:\tlast_buffer: %f\tbuffer_level: %f, choice: %lu, should_trigger_abort: %s\n",isVideo ? "video" : "audio",(double)lastBufferFill/100 , (double)bufferFill/100, i, this->shouldAbort ? "YES" : "NO");
    this->lastBufferFill = bufferFill;

}

void BufferBasedAdaptation::bitrateUpdate(uint64_t bps, uint32_t segNum)
{
}

void BufferBasedAdaptation::dLTimeUpdate(double time)
{
}

void BufferBasedAdaptation::onEOS(bool value)
{
    this->bufferEOS = value;
}

void BufferBasedAdaptation::checkedByDASHReceiver()
{
}

void BufferBasedAdaptation::bufferUpdate(uint32_t bufferFill, int maxC)
{
    this->setBitrate(bufferFill);
    this->notifyBitrateChange();
}
