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

#include "Panda.h"
#include<stdio.h>


using namespace dash::mpd;
using namespace libdash::framework::adaptation;
using namespace libdash::framework::input;
using namespace libdash::framework::mpd;
using namespace viper::managers;

PandaAdaptation::PandaAdaptation(StreamType type, MPDWrapper *mpdWrapper, struct AdaptationParameters *params) :
    AbstractAdaptationLogic   (type, mpdWrapper)
{
    this->param_Alpha = params->Panda_Alpha;
    this->param_Beta = params->Panda_Beta;
    this->param_Bmin = params->Panda_Bmin;
    this->param_K = params->Panda_K;
    this->param_W = params->Panda_W;
    this->param_Epsilon = params->Panda_Epsilon;

    this->segmentDuration = params->segmentDuration;
    this->bufferMaxSizeSeconds = params->segmentBufferSize * this->segmentDuration;
    this->targetBw = 0;
    this->targetInterTime = 0.0;

    this->averageBw = 0;
    this->smoothBw = 0;
    this->instantBw = 0;
    this->targetBw = 0;

    this->targetInterTime = 0.0;
    this->interTime = 0.0;

    this->alpha_ewma = 0.8;

    this->bufferLevel = 0;
    this->bufferLevelSeconds = 0.0;

    this->downloadTime = 0.0;

//    this->mpd = mpd;
//    this->adaptationSet = adaptationSet;
//    this->period = period;
    this->multimediaManager = NULL;
    this->representation = NULL;
    this->currentBitrate = 0;
    this->current = 0;

    // Retrieve the available bitrates
    this->mpdWrapper->acquireLock();
    std::vector<IRepresentation* > representations = this->mpdWrapper->getRepresentations(this->type);

    this->availableBitrates.clear();
    Debug("PANDA Available Bitrates...\n");
    for(size_t i = 0; i < representations.size(); i++)
    {
        this->availableBitrates.push_back((uint64_t)(representations.at(i)->GetBandwidth()));
        Debug("%d  -  %I64u bps\n", i+1, this->availableBitrates[i]);
    }

    this->representation = representations.at(0);
    this->currentBitrate = (uint64_t) this->representation->GetBandwidth();

    Debug("Panda parameters: K= %f, Bmin = %f, alpha = %f, beta = %f, W = %f\n", param_K, param_Bmin, param_Alpha, param_Beta, param_W);
}

PandaAdaptation::~PandaAdaptation() {
}

LogicType PandaAdaptation::getType()
{
    return adaptation::Panda;
}

bool PandaAdaptation::isUserDependent()
{
    return false;
}

bool PandaAdaptation::isRateBased()
{
    return true;
}

bool PandaAdaptation::isBufferBased()
{
    return true;
}

void PandaAdaptation::setMultimediaManager (viper::managers::IMultimediaManagerBase *_mmManager)
{
    this->multimediaManager = _mmManager;
}

void PandaAdaptation::notifyBitrateChange()
{
    this->mpdWrapper->setRepresentation(this->type, this->representation);
    if(this->multimediaManager->isStarted() && !this->multimediaManager->isStopping())
        if(this->type == viper::managers::StreamType::VIDEO)
            this->multimediaManager->setVideoQuality();
        else
            this->multimediaManager->setAudioQuality();
}

uint64_t PandaAdaptation::getBitrate()
{
    return this->currentBitrate;
}

void PandaAdaptation::quantizer()
{
    this->deltaUp = this->param_Epsilon * (double)this->smoothBw;
    this->deltaDown = 0.0;

    Debug("** DELTA UP:\t%f\n", this->deltaUp);

    uint64_t smoothBw_UP = this->smoothBw - this->deltaUp;
    uint64_t smoothBw_DOWN = this->smoothBw - this->deltaDown;

    Debug("** Smooth-BW UP:\t%d\t Smooth-BW DOWN:\t%d\n", smoothBw_UP, smoothBw_DOWN);

    std::vector<IRepresentation *> representations;
    representations = this->mpdWrapper->getRepresentations(this->type);
    uint32_t numQualLevels = representations.size();

    // We have to find bitrateMin and bitrateMax
    uint64_t bitrateDown, bitrateUp;

    // DOWN
    uint32_t iDown = 0;
    uint32_t i_d,i_u;
    for (i_d = 0; i_d < this->availableBitrates.size(); ++i_d) {
        if (this->availableBitrates[i_d] > smoothBw_DOWN) {
            break;
        }
    }
    if(i_d > 0)
        iDown = i_d-1;
    else
        iDown = 0;

    bitrateDown = (uint64_t) representations.at(iDown)->GetBandwidth();
    Debug("** Bitrate DOWN:\t%d\t at Quality:\t%d\n", bitrateDown, iDown);

    // UP
    uint32_t iUp = 0;
    for (i_u = 0; i_u < this->availableBitrates.size(); ++i_u) {
        if (this->availableBitrates[i_u] > smoothBw_UP) {
            break;
        }
    }
    if(i_u > 0)
        iUp = i_u-1;
    else
        iUp = 0;

    bitrateUp = (uint64_t) representations.at(iUp)->GetBandwidth();
    Debug("** Bitrate UP:\t%d\t at Quality:\t%d\n", bitrateUp, iUp);

    Debug("** Current RATE:\t%d\n Current QUALITY:\t%d\n", this->currentBitrate, this->current);


    // Next bitrate computation
    if(this->currentBitrate < bitrateUp)
    {
        this->currentBitrate = bitrateUp;
        this->current = iUp;
    }
    else if(this->currentBitrate <= bitrateDown && this->currentBitrate >= bitrateUp)
    {
        Debug("** CURRENT UNCHANGED **\n");
    }
    else
    {
        this->currentBitrate = bitrateDown;
        this->current = iDown;
    }
    this->representation = representations.at(this->current);
}

void PandaAdaptation::setBitrate(uint64_t bps)
{

    // 1. Calculating the targetBW
    if(this->targetBw)
    {
        //this->targetBw = this->targetBw + param_K * this->interTime * (param_W - ((this->targetBw - bps + this->param_W) > 0 ? this->targetBw - bps + this->param_W: 0));
        if ((double)this->targetBw - (double)bps + this->param_W > 0)
            this->targetBw = this->targetBw + (uint64_t)(param_K * this->interTime * (param_W - ((double)this->targetBw - (double)bps + this->param_W)));
        else
            this->targetBw = this->targetBw + (uint64_t)(param_K * this->interTime * param_W);
    }
    else
        this->targetBw = bps;

    Debug("** INSTANTANEOUS BW:\t%d\n", bps);
    Debug("** CLASSIC EWMA BW:\t%d\n", this->averageBw);
    Debug("** PANDA TARGET BW:\t%d\n", this->targetBw);

    // 2. Calculating the smoothBW
    if(this->interTime)
        this->smoothBw = (uint64_t)((double)this->smoothBw - this->param_Alpha * this->interTime * ((double)this->smoothBw - (double)this->targetBw));
    else
        this->smoothBw = this->targetBw;

    Debug("** PANDA SMOOTH BW:\t%d\n", this->smoothBw);

    // 3. Quantization
    this->quantizer();
    Debug("ADAPTATION_LOGIC:\tFor %s:\tlast_buffer: %f\tbuffer_level: %f, instantaneousBw: %lu, AverageBW: %lu, choice: %d\n",(this->type == viper::managers::StreamType::VIDEO) ? "video" : "audio",(double)lastBufferLevel/100 , (double)bufferLevel/100, this->instantBw, this->averageBw , this->current);
    this->lastBufferLevel = this->bufferLevel;

    // 4. Computing the "actual inter time"
    this->bufferLevelSeconds = (double)((this->bufferLevel * this->bufferMaxSizeSeconds) *1./100);
    this->targetInterTime = ((double)this->currentBitrate * segmentDuration) * 1./this->smoothBw + param_Beta * (this->bufferLevelSeconds - param_Bmin);
    Debug("** TARGET INTER TIME:\t%f\n", this->targetInterTime);
    Debug("** DOWNLOAD TIME:\t%f\n", this->downloadTime);
    this->targetInterTime = (this->targetInterTime > 0) ? this->targetInterTime : 0.0;
    this->interTime = this->targetInterTime > this->downloadTime ? this->targetInterTime : this->downloadTime;
    this->interTime = this->interTime > 3 ? 3 : this->interTime;

    Debug("** ACTUAL INTER TIME:\t%f\n", this->interTime);
    this->multimediaManager->setTargetDownloadingTime((this->type == viper::managers::StreamType::VIDEO), interTime);
}

void PandaAdaptation::bitrateUpdate(uint64_t bps, uint32_t segNum)
{
    this->instantBw = bps;

    // Avg bandwidth estimate with EWMA
    if(this->averageBw == 0)
    {
        this->averageBw = bps;
    }
    else
    {
        this->averageBw = this->alpha_ewma*this->averageBw + (1 - this->alpha_ewma)*bps;
    }
    this->mpdWrapper->acquireLock();
    this->setBitrate(bps);
    this->notifyBitrateChange();
    this->mpdWrapper->releaseLock();
}

void PandaAdaptation::dLTimeUpdate(double time)
{
    this->downloadTime = time;
}

void PandaAdaptation::bufferUpdate(uint32_t bufferfill, int maxC)
{
    Debug("bufferlvl: %d\n", bufferfill);
    this->bufferLevel = bufferfill;
}

void PandaAdaptation::onEOS(bool value)
{
}

void PandaAdaptation::checkedByDASHReceiver()
{
}
