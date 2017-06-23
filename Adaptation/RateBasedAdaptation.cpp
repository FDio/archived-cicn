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

#include "RateBasedAdaptation.h"
#include<stdio.h>
using namespace dash::mpd;
using namespace libdash::framework::adaptation;
using namespace libdash::framework::input;
using namespace libdash::framework::mpd;
using namespace viper::managers;

RateBasedAdaptation::RateBasedAdaptation(StreamType type, MPDWrapper *mpdWrapper, struct AdaptationParameters *params) :
    AbstractAdaptationLogic(type, mpdWrapper)
{
    this->mpdWrapper->acquireLock();
    std::vector<IRepresentation* > representations = this->mpdWrapper->getRepresentations(type);

    this->availableBitrates.clear();
    for(size_t i = 0; i < representations.size(); i++)
    {
        this->availableBitrates.push_back((uint64_t)(representations.at(i)->GetBandwidth()));
    }
    this->currentBitrate = this->availableBitrates.at(0);
    this->representation = representations.at(0);
    this->mpdWrapper->releaseLock();
    this->multimediaManager = NULL;
    this->alpha = params->Rate_Alpha;
    Debug("RateBasedParams:\t%f\n",alpha);
    this->averageBw = 0;
}

RateBasedAdaptation::~RateBasedAdaptation()
{
}

LogicType RateBasedAdaptation::getType()
{
    return adaptation::RateBased;
}

bool RateBasedAdaptation::isUserDependent()
{
    return false;
}

bool RateBasedAdaptation::isRateBased()
{
    return true;
}

bool RateBasedAdaptation::isBufferBased()
{
    return false;
}

void RateBasedAdaptation::setMultimediaManager(viper::managers::IMultimediaManagerBase *_mmManager)
{
    this->multimediaManager = _mmManager;
}

void RateBasedAdaptation::notifyBitrateChange()
{
    this->mpdWrapper->setRepresentation(this->type, this->representation);
    if(this->multimediaManager->isStarted() && !this->multimediaManager->isStopping())
        if(this->type == viper::managers::StreamType::VIDEO)
            this->multimediaManager->setVideoQuality();
        else
            this->multimediaManager->setAudioQuality();
}

uint64_t RateBasedAdaptation::getBitrate()
{
    return this->currentBitrate;
}

void RateBasedAdaptation::setBitrate(uint64_t bps)
{
    std::vector<IRepresentation *> representations;
    representations = this->mpdWrapper->getRepresentations(this->type);
    size_t i = 0;
    this->ewma(bps);
    for(i = 0;i < representations.size();i++)
    {
        if(representations.at(i)->GetBandwidth() > this->averageBw)
        {
            if(i > 0)
                i--;
            break;
        }
    }
    if((size_t)i == (size_t)(representations.size()))
        i = i-1;

    Debug("ADAPTATION_LOGIC:\tFor %s:\tBW_estimation(ewma): %lu, choice: %lu\n", ((this->type == viper::managers::StreamType::VIDEO) ? "video" : "audio"), this->averageBw, i);
    this->representation = representations.at(i);
    this->currentBitrate = this->representation->GetBandwidth();
}

void RateBasedAdaptation::bitrateUpdate(uint64_t bps, uint32_t segNum)
{
    Debug("Rate Based adaptation: speed received: %lu\n", bps);
    this->mpdWrapper->acquireLock();
    this->setBitrate(bps);
    this->notifyBitrateChange();
    this->mpdWrapper->releaseLock();
}

void RateBasedAdaptation::ewma(uint64_t bps)
{
    if(averageBw)
    {
        averageBw = alpha*averageBw + (1-alpha)*bps;
    }
    else
    {
        averageBw = bps;
    }
}

void RateBasedAdaptation::onEOS(bool value)
{
}

void RateBasedAdaptation::checkedByDASHReceiver()
{
}

void RateBasedAdaptation::bufferUpdate(uint32_t bufferfill, int maxC)
{
}

void RateBasedAdaptation::dLTimeUpdate(double time)
{
}
