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

#include "Bola.h"
#include <stdio.h>
#include <math.h>
#include <chrono>
#include <string>
#include <stdint.h>
#include <iostream>
#include <sstream>
#include <chrono>
#include <inttypes.h>
#include <stdlib.h>
#include <stdarg.h>
#include <algorithm>
#include <inttypes.h>
#include <time.h>
#include <limits.h>
#include <errno.h>

const double MINIMUM_BUFFER_LEVEL_SPACING = 5.0;			// The minimum space required between buffer levels (in seconds).
const uint32_t THROUGHPUT_SAMPLES = 3;						// Number of samples considered for throughput estimate.
const double SAFETY_FACTOR = 0.9;							// Safety factor used with bandwidth estimate.

using namespace dash::mpd;
using namespace libdash::framework::adaptation;
using namespace libdash::framework::input;
using namespace libdash::framework::mpd;

using std::bind;
using std::placeholders::_1;
using std::placeholders::_2;


using duration_in_seconds = std::chrono::duration<double, std::ratio<1, 1> >;

BolaAdaptation::BolaAdaptation(viper::managers::StreamType type, MPDWrapper *mpdWrapper, struct AdaptationParameters *params) :
                  AbstractAdaptationLogic   (type, mpdWrapper)
{
    this->segmentDuration = params->segmentDuration;
	this->bufferMaxSizeSeconds =(double) params->segmentBufferSize * this->segmentDuration;
	this->alphaRate = params->Bola_Alpha;
	this->bufferTargetSeconds = params->Bola_bufferTargetSeconds;

	// Set Bola init STATE
	this->initState = true;
	this->bolaState = STARTUP;

	this->lastDownloadTimeInstant = 0.0;
	this->currentDownloadTimeInstant = 0.0;
	//this->lastSegmentDownloadTime = 0.0;
	this->currentQuality = 0;
        this->representation = NULL;
	this->bufferTargetPerc = (uint32_t) ( round(this->bufferTargetSeconds / this->bufferMaxSizeSeconds)*100 );

	this->mpdWrapper->acquireLock();
	/// Retrieve available bitrates
	std::vector<IRepresentation* > representations = this->mpdWrapper->getRepresentations(this->type);

	this->availableBitrates.clear();
	Debug("BOLA Available Bitrates...\n");
	for(size_t i = 0; i < representations.size(); i++)
	{
		this->availableBitrates.push_back((uint64_t)(representations.at(i)->GetBandwidth()));
		Debug("%d  -  %I64u bps\n", i+1, this->availableBitrates[i]);
	}
	// Check if they are in increasing order (i.e., bitrate[0] <= bitrate[1], etc.)

	this->bitrateCount = this->availableBitrates.size();

	// We check if we have only one birate value or if the bitrate list is irregular (i.e., it is not strictly increasing)
	if (this->bitrateCount < 2 || this->availableBitrates[0] >= this->availableBitrates[1] || this->availableBitrates[this->bitrateCount - 2] >= this->availableBitrates[this->bitrateCount - 1]) {
		this->bolaState = ONE_BITRATE;
	    // return 0;   // Check if exit with a message is necessary
	}

    this->totalDuration = TimeResolver::getDurationInSec(this->mpdWrapper->getMediaPresentationDuration());
	if(!(this->segmentDuration))
	    this->segmentDuration = 2.0;
	Debug("Total Duration - BOLA:\t%f\nSegment Duration - BOLA:\t%f\n",this->totalDuration, this->segmentDuration);
	// if not correct --> segmentDuration = 2.0;

	// Compute the BOLA Buffer Target
	this->bolaBufferTargetSeconds = this->bufferTargetSeconds;
	if (this->bolaBufferTargetSeconds < this->segmentDuration + MINIMUM_BUFFER_LEVEL_SPACING)
	{
		this->bolaBufferTargetSeconds = this->segmentDuration + MINIMUM_BUFFER_LEVEL_SPACING;
	}
	Debug("BOLA Buffer Target Seconds:\t%f\n",this->bolaBufferTargetSeconds);

	// Compute UTILTY vector, Vp, and gp
	Debug("BOLA Utility Values...\n");
	for (uint32_t i = 0; i < this->bitrateCount; ++i) {
		this->utilityVector.push_back( log(((double)this->availableBitrates[i] * (1./(double)this->availableBitrates[0]))));
		Debug("%d  -  %f\n", i+1, this->utilityVector[i]);
	}

	this->Vp = (this->bolaBufferTargetSeconds - this->segmentDuration) / this->utilityVector[this->bitrateCount - 1];
	this->gp = 1.0 + this->utilityVector[this->bitrateCount - 1] / (this->bolaBufferTargetSeconds / this->segmentDuration - 1.0);

	Debug("BOLA Parameters:\tVp:  %f\tgp:  %f\n",this->Vp, this->gp);
	/* If bufferTargetSeconds (not bolaBufferTargetSecond) is large enough, we might guarantee that Bola will never rebuffer
	 * unless the network bandwidth drops below the lowest encoded bitrate level. For this to work, Bola needs to use the real buffer
	 * level without the additional virtualBuffer. Also, for this to work efficiently, we need to make sure that if the buffer level
	 * drops to one fragment during a download, the current download does not have more bits remaining than the size of one fragment
	 * at the lowest quality*/

	this->maxRtt = 0.2; // Is this reasonable?
	if(this->bolaBufferTargetSeconds == this->bufferTargetSeconds) {
		this->safetyGuarantee = true;
	}
	if (this->safetyGuarantee) {
		Debug("BOLA SafetyGuarantee...\n");
		// we might need to adjust Vp and gp
		double VpNew = this->Vp;
		double gpNew = this->gp;
		for (uint32_t i = 1; i < this->bitrateCount; ++i) {
			double threshold = VpNew * (gpNew - this->availableBitrates[0] * this->utilityVector[i] / (this->availableBitrates[i] - this->availableBitrates[0]));
			double minThreshold = this->segmentDuration * (2.0 - this->availableBitrates[0] / this->availableBitrates[i]) + maxRtt;
			if (minThreshold >= this->bufferTargetSeconds) {
				safetyGuarantee = false;
				break;
			}
			if (threshold < minThreshold) {
				VpNew = VpNew * (this->bufferTargetSeconds - minThreshold) / (this->bufferTargetSeconds - threshold);
				gpNew = minThreshold / VpNew + this->utilityVector[i] * this->availableBitrates[0] / (this->availableBitrates[i] - this->availableBitrates[0]);
			}
		}
		if (safetyGuarantee && (this->bufferTargetSeconds - this->segmentDuration) * VpNew / this->Vp < MINIMUM_BUFFER_LEVEL_SPACING) {
			safetyGuarantee = false;
		}
		if (safetyGuarantee) {
			this->Vp = VpNew;
			this->gp = gpNew;
		}
	}

	Debug("BOLA New Parameters:\tVp:  %f\tgp:  %f\n",this->Vp, this->gp);

	// Capping of the virtual buffer (when using it)
	this->bolaBufferMaxSeconds = this->Vp * (this->utilityVector[this->bitrateCount - 1] + this->gp);
	Debug("BOLA Max Buffer Seconds:\t%f\n",this->bolaBufferMaxSeconds);

	this->virtualBuffer = 0.0;			// Check if we should use either the virtualBuffer or the safetyGuarantee

	this->instantBw = 0;
	this->averageBw = 0;
	this->batchBw = 0;					// Computed every THROUGHPUT_SAMPLES samples (average)
	this->batchBwCount = 0;

	this->multimediaManager = NULL;
	this->lastBufferFill = 0;		// (?)
	this->bufferEOS = false;
	this->shouldAbort = false;
	this->isCheckedForReceiver = false;

	this->representation = representations.at(0);
	this->currentBitrate = (uint64_t) this->representation->GetBandwidth();

	this->mpdWrapper->releaseLock();
	Debug("BOLA Init Params - \tAlpha: %f \t BufferTarget: %f\n",this->alphaRate, this->bufferTargetSeconds);
	Debug("BOLA Init Current BitRate - %I64u\n",this->currentBitrate);
	Debug("Buffer Adaptation BOLA:	STARTED\n");
}

BolaAdaptation::~BolaAdaptation()
{
}

LogicType BolaAdaptation::getType()
{
    return adaptation::BufferBased;
}

bool BolaAdaptation::isUserDependent()
{
	return false;
}

bool BolaAdaptation::isRateBased()
{
	return true;
}
bool BolaAdaptation::isBufferBased()
{
	return true;
}

void BolaAdaptation::setMultimediaManager(viper::managers::IMultimediaManagerBase *_mmManager)
{
	this->multimediaManager = _mmManager;
}

void BolaAdaptation::notifyBitrateChange()
{
	this->mpdWrapper->setRepresentation(this->type, this->representation);
	if(this->multimediaManager)
        if(this->multimediaManager->isStarted() && !this->multimediaManager->isStopping())
			if(type==viper::managers::VIDEO)
                this->multimediaManager->setVideoQuality();
//                this->multimediaManager->setVideoQuality(this->period, this->adaptationSet, this->representation);
			else
                this->multimediaManager->setAudioQuality();
//                this->multimediaManager->setAudioQuality(this->period, this->adaptationSet, this->representation);
	//Should Abort is done here to avoid race condition with DASHReceiver::DoBuffering()
	if(this->shouldAbort)
	{
        this->multimediaManager->shouldAbort(1);
//        this->multimediaManager->shouldAbort((this->type == viper::managers::StreamType::VIDEO));
	}
	this->shouldAbort = false;
}

uint64_t BolaAdaptation::getBitrate()
{
	return this->currentBitrate;
}

int BolaAdaptation::getQualityFromThroughput(uint64_t bps) {
    int q = 0;
    for (int i = 0; i < this->availableBitrates.size(); ++i) {
        if (this->availableBitrates[i] > bps) {
            break;
        }
        q = i;
    }
    return q;
}

int BolaAdaptation::getQualityFromBufferLevel(double bufferLevelSec) {
	int quality = this->bitrateCount - 1;
	double score = 0.0;
	for (int i = 0; i < this->bitrateCount; ++i) {
		double s = (this->utilityVector[i] + this->gp - bufferLevelSec / this->Vp) / this->availableBitrates[i];
		if (s > score) {
			score = s;
			quality = i;
		}
	}
	return quality;
}

void BolaAdaptation::setBitrate(uint32_t bufferFill)
{
	// *** NB *** Insert Log messages

	if(this->initState)
	{
		this->initState = false;

		if(this->bolaState != ONE_BITRATE)
		{
		    if(this->batchBw != 0)		// Check the current estimated throughput (batch mean)
		    	this->currentQuality = getQualityFromThroughput(this->batchBw*SAFETY_FACTOR);
			//else --> quality unchanged
		}
		//this->representation = this->availableBitrates[this->currentQuality];
		//this->currentBitrate = (uint64_t) this->representation->GetBandwidth();
		this->representation = this->mpdWrapper->getRepresentationAt(this->type, this->currentQuality);
		this->currentBitrate = (uint64_t) this->availableBitrates[this->currentQuality];
		Debug("INIT - Current Bitrate:\t%I64u\n", this->currentBitrate);
		Debug("ADAPTATION_LOGIC:\tFor %s:\tlast_buffer: %f\tbuffer_level: %f, instantaneousBw: %lu, AverageBW: %lu, choice: %d\n",(this->type == viper::managers::VIDEO) ? "video" : "audio",(double)lastBufferFill/100 , (double)bufferFill/100, this->instantBw, this->averageBw , this->currentQuality);
		this->lastBufferFill = bufferFill;
		return;
	}

	if(this->bolaState == ONE_BITRATE) {
		this->currentQuality = 0;
		//this->representation = this->availableBitrates[this->currentQuality];
		//this->currentBitrate = (uint64_t) this->representation->GetBandwidth();
		this->representation = this->mpdWrapper->getRepresentationAt(this->type,this->currentQuality);
		this->currentBitrate = (uint64_t) this->availableBitrates[this->currentQuality];
		Debug("ONE BITRATE - Current Bitrate:\t%I64u\n", this->currentBitrate);
		Debug("ADAPTATION_LOGIC:\tFor %s:\tlast_buffer: %f\tbuffer_level: %f, instantaneousBw: %lu, AverageBW: %lu, choice: %d\n",(this->type == viper::managers::VIDEO) ? "video" : "audio",(double)lastBufferFill/100 , (double)bufferFill/100, this->instantBw, this->averageBw , this->currentQuality);
		this->lastBufferFill = bufferFill;
		return;
	}

	// Obtain bufferFill in seconds;
	double bufferLevelSeconds = (double)( (bufferFill * this->bufferMaxSizeSeconds) *1./100);
	int bolaQuality = getQualityFromBufferLevel(bufferLevelSeconds);

	Debug("REGULAR - Buffer Level Seconds:\t%f; Bola Quality:\t%d\n", bufferLevelSeconds, bolaQuality);


	if (bufferLevelSeconds <= 0.1) {
		// rebuffering occurred, reset virtual buffer
		this->virtualBuffer = 0.0;
	}

	// We check if the safetyGuarantee should be used. if not, we use the virtual buffer
	// STILL NOT COMPLETE; Find a way to retrieved time since the last download
	if (!this->safetyGuarantee) // we can use virtualBuffer
	{
		// find out if there was delay because of lack of availability or because bolaBufferTarget > bufferTarget
		// TODO
	    //double timeSinceLastDownload = getDelayFromLastFragmentInSeconds();		// Define function
	    double timeSinceLastDownload = this->currentDownloadTimeInstant - this->lastDownloadTimeInstant;

	    qDebug("VirtualBuffer - Time Since Last Download:\t%f", timeSinceLastDownload);

	    if (timeSinceLastDownload > 0.0) {
	    	this->virtualBuffer += timeSinceLastDownload;
	    }
	    if ( (bufferLevelSeconds + this->virtualBuffer) > this->bolaBufferMaxSeconds) {
	    	this->virtualBuffer = this->bolaBufferMaxSeconds - bufferLevelSeconds;
	    }
	    if (this->virtualBuffer < 0.0) {
	    	this->virtualBuffer = 0.0;
	    }

	    qDebug("VirtualBuffer - Virtual Buffer Value:\t%f", this->virtualBuffer);

	    // Update currentDownloadTimeInstant
	    this->lastDownloadTimeInstant = this->currentDownloadTimeInstant;

	    // Update bolaQuality using virtualBuffer: bufferLevel might be artificially low because of lack of availability

	    int bolaQualityVirtual = getQualityFromBufferLevel(bufferLevelSeconds + this->virtualBuffer);
	    qDebug("VirtualBuffer - Bola Quality Virtual:\t%d", bolaQualityVirtual);
	    if (bolaQualityVirtual > bolaQuality) {
	    	// May use quality higher than that indicated by real buffer level.
	    	// In this case, make sure there is enough throughput to download a fragment before real buffer runs out.
	    	int maxQuality = bolaQuality;
	    	while (maxQuality < bolaQualityVirtual && (this->availableBitrates[maxQuality + 1] * this->segmentDuration) / (this->currentBitrate * SAFETY_FACTOR) < bufferLevelSeconds)
	    	{
	    		++maxQuality;
	    	}
	    	// TODO: maybe we can use a more conservative level here, but this should be OK
	    	Debug("VirtualBuffer - Bola Quality Virtual HIGHER than Bola Quality - Max Quality:\t%d\n", maxQuality);
	    	if (maxQuality > bolaQuality)
	    	{
	    		// We can (and will) download at a quality higher than that indicated by real buffer level.
	    		if (bolaQualityVirtual <= maxQuality) {
	    			// we can download fragment indicated by real+virtual buffer without rebuffering
	    			bolaQuality = bolaQualityVirtual;
	    		} else {
	    			// downloading fragment indicated by real+virtual rebuffers, use lower quality
	    			bolaQuality = maxQuality;
	    			// deflate virtual buffer to match quality
	    			double targetBufferLevel = this->Vp * (this->gp + this->utilityVector[bolaQuality]);
	    			if (bufferLevelSeconds + this->virtualBuffer > targetBufferLevel) {
	    				this->virtualBuffer = targetBufferLevel - bufferLevelSeconds;
	    				if (this->virtualBuffer < 0.0) { // should be false
	    					this->virtualBuffer = 0.0;
	    				}
	    			}
	    		}
	    	}
	    }
	}


	if (this->bolaState == STARTUP || this->bolaState == STARTUP_NO_INC) {
		// in startup phase, use some throughput estimation

	    int quality = getQualityFromThroughput(this->batchBw*SAFETY_FACTOR);

	    if (this->batchBw <= 0.0) {
	    	// something went wrong - go to steady state
	    	this->bolaState = STEADY;
	    }
	    if (this->bolaState == STARTUP && quality < this->currentQuality) {
	    	// Since the quality is decreasing during startup, it will not be allowed to increase again.
	    	this->bolaState = STARTUP_NO_INC;
	    }
	    if (this->bolaState == STARTUP_NO_INC && quality > this->currentQuality) {
	    	// In this state the quality is not allowed to increase until steady state.
	    	quality = this->currentQuality;
	    }
	    if (quality <= bolaQuality) {
	    	// Since the buffer is full enough for steady state operation to match startup operation, switch over to steady state.
	    	this->bolaState = STEADY;
	    }
	    if (this->bolaState != STEADY) {
	    	// still in startup mode
	    	this->currentQuality = quality;
	    	//this->representation = this->availableBitrates[this->currentQuality];
	    	//this->currentBitrate = (uint64_t) this->representation->GetBandwidth();
	    	this->representation = this->mpdWrapper->getRepresentationAt(this->type, this->currentQuality);
	    	this->currentBitrate = (uint64_t) this->availableBitrates[this->currentQuality];
	    	Debug("STILL IN STARTUP - Current Bitrate:\t%I64u\n", this->currentBitrate);
	    	Debug("ADAPTATION_LOGIC:\tFor %s:\tlast_buffer: %f\tbuffer_level: %f, instantaneousBw: %lu, AverageBW: %lu, choice: %d\n",(this->type == viper::managers::VIDEO) ? "video" : "audio",(double)lastBufferFill/100 , (double)bufferFill/100, this->instantBw, this->averageBw , this->currentQuality);
	    	this->lastBufferFill = bufferFill;
	    	return;
	    }
	}

	// Steady State

	// In order to avoid oscillation, the "BOLA-O" variant is implemented.
	// When network bandwidth lies between two encoded bitrate levels, stick to the lowest one.
	double delaySeconds = 0.0;
	if (bolaQuality > this->currentQuality) {
		Debug("STEADY -- BOLA QUALITY:\t%d  - HIGHER than - CURRENT QUALITY:\t%I64u\n", bolaQuality, this->currentBitrate);
		// do not multiply throughput by bandwidthSafetyFactor here;
		// we are not using throughput estimation but capping bitrate to avoid oscillations
		int quality = getQualityFromThroughput(this->batchBw);
		if (bolaQuality > quality) {
			// only intervene if we are trying to *increase* quality to an *unsustainable* level
			if (quality < this->currentQuality) {
				// The aim is only to avoid oscillations - do not drop below current quality
				quality = this->currentQuality;
			} else {
				// We are dropping to an encoded bitrate which is a little less than the network bandwidth
				// since bitrate levels are discrete. Quality 'quality' might lead to buffer inflation,
				// so we deflate the buffer to the level that 'quality' gives positive utility.
				double targetBufferLevel = this->Vp * (this->utilityVector[quality] + this->gp);
				delaySeconds = bufferLevelSeconds - targetBufferLevel;
			}
			bolaQuality = quality;
		}
	}

	if (delaySeconds > 0.0) {
		// first reduce virtual buffer
		if (delaySeconds > this->virtualBuffer) {
			delaySeconds -= this->virtualBuffer;
			this->virtualBuffer = 0.0;
		} else {
			this->virtualBuffer -= delaySeconds;
			delaySeconds = 0.0;
		}
	}
	if (delaySeconds > 0.0) {
		// TODO  Check the scope of this function. Is it a delayed request?
		// streamProcessor.getScheduleController().setTimeToLoadDelay(1000.0 * delaySeconds);
		// NEED TO CHECK THIS
		Debug("STEADY -- DELAY DOWNLOAD OF:\t%f\n", delaySeconds);
        this->multimediaManager->setTargetDownloadingTime((this->type == viper::managers::StreamType::VIDEO), delaySeconds);
	}

	this->currentQuality = bolaQuality;
	//this->representation = this->availableBitrates[this->currentQuality];
	this->representation = this->mpdWrapper->getRepresentationAt(this->type, this->currentQuality);
	this->currentBitrate = (uint64_t) this->availableBitrates[this->currentQuality];
	Debug("STEADY - Current Bitrate:\t%I64u\n", this->currentBitrate);
	Debug("ADAPTATION_LOGIC:\tFor %s:\tlast_buffer: %f\tbuffer_level: %f, instantaneousBw: %lu, AverageBW: %lu, choice: %d\n",(this->type == viper::managers::VIDEO) ? "video" : "audio",(double)lastBufferFill/100 , (double)bufferFill/100, this->instantBw, this->averageBw , this->currentQuality);
	this->lastBufferFill = bufferFill;
}

void BolaAdaptation::bitrateUpdate(uint64_t bps, uint32_t segNum)
{
	this->instantBw = bps;

	// Avg bandwidth estimate with EWMA
	if(this->averageBw == 0)
	{
		this->averageBw = bps;
	}
	else
	{
		this->averageBw = this->alphaRate*this->averageBw + (1 - this->alphaRate)*bps;
	}

	// Avg bandwidth estimate with batch mean of THROUGHPUT_SAMPLES sample
	this->batchBwCount++;
	this->batchBwSamples.push_back(bps);

	if(this->batchBwCount++ == THROUGHPUT_SAMPLES)
	{
		for(int i=0; i<THROUGHPUT_SAMPLES; i++)
			this->batchBw += this->batchBwSamples[i];

		this->batchBw /= THROUGHPUT_SAMPLES;

		Debug("BATCH BW:\t%I64u\n", this->batchBw);

		this->batchBwCount=0;
		this->batchBwSamples.clear();
	}
}

void BolaAdaptation::dLTimeUpdate(double time)
{
	auto m_now = std::chrono::system_clock::now();
	auto m_now_sec = std::chrono::time_point_cast<std::chrono::seconds>(m_now);

	auto now_value = m_now_sec.time_since_epoch();
	double dl_instant = now_value.count();
	//this->lastSegmentDownloadTime = time;
	//this->currentDownloadTimeInstant = std::chrono::duration_cast<duration_in_seconds>(system_clock::now()).count();
	this->currentDownloadTimeInstant = dl_instant;
}

void BolaAdaptation::onEOS(bool value)
{
	this->bufferEOS = value;
}

void BolaAdaptation::checkedByDASHReceiver()
{
	this->isCheckedForReceiver = false;
}
void BolaAdaptation::bufferUpdate(uint32_t bufferFill, int maxC)
{
    this->mpdWrapper->acquireLock();
    this->setBitrate(bufferFill);
    this->notifyBitrateChange();
    this->mpdWrapper->releaseLock();
}
