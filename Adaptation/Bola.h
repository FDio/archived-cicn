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

#ifndef LIBDASH_FRAMEWORK_ADAPTATION_BOLA_H_
#define LIBDASH_FRAMEWORK_ADAPTATION_BOLA_H_

#include "AbstractAdaptationLogic.h"
#include "../MPD/AdaptationSetStream.h"
#include "../Input/IDASHReceiverObserver.h"

namespace libdash
{
namespace framework
{
namespace adaptation
{
class BolaAdaptation : public AbstractAdaptationLogic
{
public:
    BolaAdaptation            (dash::mpd::IMPD *mpd, dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet, bool isVid, struct AdaptationParameters *params);
    virtual ~BolaAdaptation();

    virtual LogicType getType();
    virtual bool isUserDependent();
    virtual bool isRateBased();
    virtual bool isBufferBased();
    virtual void bitrateUpdate(uint64_t bps, uint32_t segNum);
    virtual void dLTimeUpdate(double time);
    virtual void bufferUpdate(uint32_t bufferFill, int maxC);
    void setBitrate(uint32_t bufferFill);
    uint64_t getBitrate();
    virtual void setMultimediaManager(viper::managers::IMultimediaManagerBase *_mmManager);
    void notifyBitrateChange();
    void onEOS(bool value);
    void checkedByDASHReceiver();

    int getQualityFromThroughput(uint64_t bps);
    int getQualityFromBufferLevel(double bufferLevelSec);

private:
    enum BolaState
    {
        ONE_BITRATE,			// If one bitrate (or init failed), always NO_CHANGE
        STARTUP,				// Download fragments at most recently measured throughput
        STARTUP_NO_INC,			// If quality increased then decreased during startup, then quality cannot be increased.
        STEADY					// The buffer is primed (should be above bufferTarget)
    };

    bool							initState;
    double 							bufferMaxSizeSeconds;		// Usually set to 30s
    double 							bufferTargetSeconds;  	// It is passed as an init parameter.
    // It states the difference between STARTUP and STEADY
    // 12s following dash.js implementation

    double 							bolaBufferTargetSeconds; 	// BOLA introduces a virtual buffer level in order to make quality decisions
    // as it was filled (instead of the actual bufferTargetSeconds)

    double 							bolaBufferMaxSeconds; 	// When using the virtual buffer, it must be capped.

    uint32_t 						bufferTargetPerc;		// Computed considering a bufferSize = 30s
    double 							totalDuration;			// Total video duration in seconds (taken from MPD)
    double 							segmentDuration;		// Segment duration in seconds

    std::vector<uint64_t>			availableBitrates;
    std::vector<double>				utilityVector;
    uint32_t						bitrateCount;			// Number of available bitrates
    BolaState						bolaState;				// Keeps track of Bola state

    // Bola Vp and gp (multiplied by the segment duration 'p')
    // They are dimensioned such that log utility would always prefer
    // - the lowest bitrate when bufferLevel = segmentDuration
    // - the highest bitrate when bufferLevel = bufferTarget
    double							Vp;
    double 							gp;

    bool							safetyGuarantee;
    double							maxRtt;

    double 							virtualBuffer;

    uint64_t						currentBitrate;
    int 							currentQuality;
    uint64_t						batchBw;
    int								batchBwCount;
    std::vector<uint64_t>			batchBwSamples;
    uint64_t						instantBw;
    uint64_t						averageBw;

    double 							lastDownloadTimeInstant;
    double 							currentDownloadTimeInstant;
    double							lastSegmentDownloadTime;

    uint32_t						lastBufferFill;
    bool							bufferEOS;
    bool							shouldAbort;
    double							alphaRate;
    bool							isCheckedForReceiver;

    viper::managers::IMultimediaManagerBase	*multimediaManager;
    dash::mpd::IRepresentation		*representation;
};
}
}
}

#endif /* LIBDASH_FRAMEWORK_ADAPTATION_BOLA_H_ */
