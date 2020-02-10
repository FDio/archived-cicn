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

#include "AdapTech.h"
#include<stdio.h>

using namespace dash::mpd;
using namespace libdash::framework::adaptation;
using namespace libdash::framework::input;
using namespace libdash::framework::mpd;

AdapTechAdaptation::AdapTechAdaptation(viper::managers::StreamType type, MPDWrapper *mpdWrapper, struct AdaptationParameters *params) :
    AbstractAdaptationLogic(type, mpdWrapper)
{
    this->alphaRate = params->Adaptech_Alpha;
    this->reservoirThreshold = params->Adaptech_FirstThreshold;
    this->maxThreshold = params->Adaptech_SecondThreshold;
    this->switchUpThreshold = params->Adaptech_SwitchUpThreshold;
    this->slackParam = params->Adaptech_SlackParameter;

    this->m_count = 0;
    this->instantBw = 0;
    this->averageBw = 0;
//    this->representation = this->adaptationSet->GetRepresentation().at(0);

    this->multimediaManager = NULL;
    this->lastBufferFill = 0;
    this->bufferEOS = false;
    this->shouldAbort = false;
    this->isCheckedForReceiver = false;
    this->myQuality = 0;
    this->currentBitrate = 0;
    qDebug("BufferRateBasedParams:\talpha:%f\tfirst threshold: %f\tsecond threshold: %f\tswitch-up margin: %d\tSlack: %f",this->alphaRate, (double)reservoirThreshold/100, (double)maxThreshold/100, this->switchUpThreshold, this->slackParam);
    qDebug("Buffer Adaptation:	STARTED");
}
AdapTechAdaptation::~AdapTechAdaptation()
{
}

LogicType AdapTechAdaptation::getType()
{
    return adaptation::BufferBased;
}

bool AdapTechAdaptation::isUserDependent()
{
    return false;
}

bool AdapTechAdaptation::isRateBased()
{
    return true;
}
bool AdapTechAdaptation::isBufferBased()
{
    return true;
}

void AdapTechAdaptation::setMultimediaManager(viper::managers::IMultimediaManagerBase *_mmManager)
{
    this->multimediaManager = _mmManager;
}

void AdapTechAdaptation::notifyBitrateChange()
{
    this->mpdWrapper->setRepresentation(this->type, this->representation);
    if(this->multimediaManager)
        if(this->multimediaManager->isStarted() && !this->multimediaManager->isStopping())
            if(this->type==viper::managers::StreamType::VIDEO)
                this->multimediaManager->setVideoQuality();
            else
                this->multimediaManager->setAudioQuality();
}

uint64_t AdapTechAdaptation::getBitrate()
{
    return this->currentBitrate;
}

void AdapTechAdaptation::setBitrate(uint32_t bufferFill)
{
    std::vector<IRepresentation *> representations;
    representations = this->mpdWrapper->getRepresentations(this->type);
    bool flagIsSet = this->mpdWrapper->getSegmentIsSetFlag(this->type);
    int mySetQuality = this->mpdWrapper->getSegmentQuality(this->type);

    if(flagIsSet)
    {
        qDebug("Adaptech:\tFor %s:\tbuffer_level: %f, instantaneousBw: %lu, AverageBW: %lu,already set: %d",(this->type == viper::managers::StreamType::VIDEO) ? "video" : "audio", (double)bufferFill/100, this->instantBw, this->averageBw , this->myQuality);

        if(bufferFill < this->reservoirThreshold)
        {
            if(mySetQuality == -1)
            {
                mySetQuality = this->myQuality;
                this->myQuality = 0;
                this->representation = representations.at(this->myQuality);
                qDebug("Adaptech:\tFor %s: buffer level too low, going to panic mode, old quality: %d",(this->type == viper::managers::StreamType::VIDEO) ? "video" : "audio", mySetQuality);
                this->mpdWrapper->setSegmentQuality(this->type, mySetQuality);
            }
        }
        else
        {
            if(mySetQuality != -1)
            {
                this->myQuality = mySetQuality;
                qDebug("AdaptechNA:\tFor %s: buffer level high enough, restoring old computed quality: %d",(this->type == viper::managers::StreamType::VIDEO) ? "video" : "audio", mySetQuality);
            }
            this->representation = representations.at(this->myQuality);
        }
    }
    else
    {
        this->setBitrateOption1(bufferFill);
        this->mpdWrapper->setSegmentIsSetFlag(this->type, true);
    }
}
void AdapTechAdaptation::setBitrateOption1(uint32_t bufferFill)
{
    uint32_t phi1, phi2;
    std::vector<IRepresentation *> representations;
    representations = this->mpdWrapper->getRepresentations(this->type);
    size_t i = 0;

    qDebug("bufferlevel: %u, instant rate %lu, average rate %lu", bufferFill, this->instantBw, this->averageBw);
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
    qDebug("ADAPTATION_LOGIC:\tFor %s:\tlast_buffer: %f\tbuffer_level: %f, instantaneousBw: %lu, AverageBW: %lu, choice: %d",(this->type == viper::managers::StreamType::VIDEO) ? "video" : "audio",(double)lastBufferFill/100 , (double)bufferFill/100, this->instantBw, this->averageBw , this->myQuality);
}

void AdapTechAdaptation::bitrateUpdate(uint64_t bps, uint32_t segNum)
{
    qDebug("rate estimation: %lu", bps);
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

void AdapTechAdaptation::onEOS(bool value)
{
    this->bufferEOS = value;
}

void AdapTechAdaptation::checkedByDASHReceiver()
{
    this->isCheckedForReceiver = false;
}
void AdapTechAdaptation::bufferUpdate(uint32_t bufferFill, int maxC)
{
    qDebug("buffer update: %u", bufferFill);
    EnterCriticalSection(&this->monitorLock);
    this->setBitrate(bufferFill);
    this->notifyBitrateChange();
    LeaveCriticalSection(&this->monitorLock);
}

void AdapTechAdaptation::dLTimeUpdate(double time)
{
}


