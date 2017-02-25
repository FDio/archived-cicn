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

#include "BufferBasedAdaptationWithRateBased.h"
#include<stdio.h>

using namespace dash::mpd;
using namespace libdash::framework::adaptation;
using namespace libdash::framework::input;
using namespace libdash::framework::mpd;

BufferBasedAdaptationWithRateBased::BufferBasedAdaptationWithRateBased(IMPD *mpd, IPeriod *period, IAdaptationSet *adaptationSet, bool isVid, struct AdaptationParameters *params) :
    AbstractAdaptationLogic(mpd, period, adaptationSet, isVid)
{
    this->alphaRate = params->Adaptech_Alpha;
    this->reservoirThreshold = params->Adaptech_FirstThreshold;
    this->maxThreshold = params->Adaptech_SecondThreshold;
    this->switchUpThreshold = params->Adaptech_SwitchUpThreshold;
    this->slackParam = params->Adaptech_SlackParameter;

    std::vector<IRepresentation* > representations = this->adaptationSet->GetRepresentation();

    this->m_count = 0;
    this->instantBw = 0;
    this->averageBw = 0;
    this->representation = this->adaptationSet->GetRepresentation().at(0);
    this->multimediaManager = NULL;
    this->lastBufferFill = 0;
    this->bufferEOS = false;
    this->shouldAbort = false;
    this->isCheckedForReceiver = false;
    this->myQuality = 0;
    Debug("BufferRateBasedParams:\talpha:%f\tfirst threshold: %f\tsecond threshold: %f\tswitch-up margin: %d\tSlack: %f\n",this->alphaRate, (double)reservoirThreshold/100, (double)maxThreshold/100, this->switchUpThreshold, this->slackParam);
    Debug("Buffer Adaptation:	STARTED\n");
}
BufferBasedAdaptationWithRateBased::~BufferBasedAdaptationWithRateBased         ()
{
}

LogicType BufferBasedAdaptationWithRateBased::getType()
{
    return adaptation::BufferBased;
}

bool BufferBasedAdaptationWithRateBased::isUserDependent()
{
    return false;
}

bool BufferBasedAdaptationWithRateBased::isRateBased()
{
    return true;
}
bool BufferBasedAdaptationWithRateBased::isBufferBased()
{
    return true;
}

void BufferBasedAdaptationWithRateBased::setMultimediaManager(viper::managers::IMultimediaManagerBase *_mmManager)
{
    this->multimediaManager = _mmManager;
}

void BufferBasedAdaptationWithRateBased::notifyBitrateChange()
{
    if(this->multimediaManager)
        if(this->multimediaManager->isStarted() && !this->multimediaManager->isStopping())
            if(this->isVideo)
                this->multimediaManager->setVideoQuality(this->period, this->adaptationSet, this->representation);
            else
                this->multimediaManager->setAudioQuality(this->period, this->adaptationSet, this->representation);
    //Should Abort is done here to avoid race condition with DASHReceiver::DoBuffering()
    if(this->shouldAbort)
    {
        this->multimediaManager->shouldAbort(this->isVideo);
    }
    this->shouldAbort = false;
}

uint64_t BufferBasedAdaptationWithRateBased::getBitrate()
{
    return this->currentBitrate;
}

void BufferBasedAdaptationWithRateBased::setBitrate(uint32_t bufferFill)
{
    uint32_t phi1, phi2;
    std::vector<IRepresentation *> representations;
    representations = this->adaptationSet->GetRepresentation();
    size_t i = 0;

    Debug("bufferlevel: %u, instant rate %lu, average rate %lu\n", bufferFill, this->instantBw, this->averageBw);
    phi1 = 0;
    phi2 = 0;
    while(i < representations.size())
    {
        if(phi1 == 0 && representations.at(i)->GetBandwidth() > slackParam * this->instantBw)
        {
            phi1 = representations.at((i == 0) ? i : i -1)->GetBandwidth();
        }
        if(phi2 == 0 && representations.at(i)->GetBandwidth() > slackParam * this->averageBw)
        {
            phi2 = representations.at((i == 0) ? i : i -1)->GetBandwidth();
        }
        i++;
    }

    if(!phi1)
        phi1 = representations.at(representations.size() - 1)->GetBandwidth();

    if(!phi2)
        phi2 = representations.at(representations.size() - 1)->GetBandwidth();

    if(bufferFill < this->reservoirThreshold)
    {
        this->m_count = 0;
        this->myQuality = 0;
    }
    else
    {
        if(bufferFill < this->maxThreshold)
        {
            this->m_count = 0;
            if(this->currentBitrate > phi1)
            {
                if(this->myQuality > 0)
                {
                    this->myQuality--;
                }
            }
            else
            {
                if(this->currentBitrate < phi1)
                {
                    if(this->myQuality < representations.size() - 1)
                    {
                        this->myQuality++;
                    }
                }
            }
        }
        else
        { // bufferFill > this->maxThreshold
            if(this->currentBitrate < phi2)
            {
                m_count++;

                if(m_count >= switchUpThreshold && this->myQuality < representations.size() - 1)
                {
                    this->m_count = 0;
                    this->myQuality++;
                }
            }
        }
    }
    this->representation = representations.at(this->myQuality);
    this->currentBitrate = (uint64_t) this->representation->GetBandwidth();
    Debug("ADAPTATION_LOGIC:\tFor %s:\tlast_buffer: %f\tbuffer_level: %f, instantaneousBw: %lu, AverageBW: %lu, choice: %d\n",isVideo ? "video" : "audio",(double)lastBufferFill/100 , (double)bufferFill/100, this->instantBw, this->averageBw , this->myQuality);
}

void BufferBasedAdaptationWithRateBased::bitrateUpdate(uint64_t bps, uint32_t segNum)
{
    Debug("rate estimation: %lu\n", bps);
    this->instantBw = bps;
    if(this->averageBw == 0)
    {
        this->averageBw = bps;
    }
    else
    {
        this->averageBw = this->alphaRate*this->averageBw + (1 - this->alphaRate)*bps;
    }
}

void BufferBasedAdaptationWithRateBased::onEOS(bool value)
{
    this->bufferEOS = value;
}

void BufferBasedAdaptationWithRateBased::checkedByDASHReceiver()
{
    this->isCheckedForReceiver = false;
}
void BufferBasedAdaptationWithRateBased::bufferUpdate(uint32_t bufferFill, int maxC)
{
    Debug("buffer update: %u\n", bufferFill);
    this->setBitrate(bufferFill);
    this->notifyBitrateChange();
}

void BufferBasedAdaptationWithRateBased::dLTimeUpdate(double time)
{
}

