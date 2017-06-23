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

#ifndef LIBDASH_FRAMEWORK_ADAPTATION_PANDA_H_
#define LIBDASH_FRAMEWORK_ADAPTATION_PANDA_H_

#include "AbstractAdaptationLogic.h"
#include "../MPD/AdaptationSetStream.h"
#include "../Input/IDASHReceiverObserver.h"

namespace libdash
{
namespace framework
{
namespace adaptation
{
class PandaAdaptation : public AbstractAdaptationLogic
{
public:
//    PandaAdaptation(dash::mpd::IMPD *mpd, dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet, bool isVid, struct AdaptationParameters *params);
    PandaAdaptation(viper::managers::StreamType type, libdash::framework::mpd::MPDWrapper *mpdWrapper, struct AdaptationParameters *params);
    virtual ~PandaAdaptation();

    virtual LogicType getType();
    virtual bool isUserDependent();
    virtual bool isRateBased();
    virtual bool isBufferBased();
    virtual void bitrateUpdate(uint64_t bps, uint32_t segNum);
    virtual void dLTimeUpdate(double time);
    virtual void bufferUpdate(uint32_t bufferFill, int maxC);
    void setBitrate(uint64_t bufferFill);
    uint64_t getBitrate();
    virtual void setMultimediaManager(viper::managers::IMultimediaManagerBase *_mmManager);
    void notifyBitrateChange();
    void onEOS(bool value);
    void checkedByDASHReceiver();

    void quantizer();
private:
    uint64_t					currentBitrate;

    std::vector<uint64_t>			availableBitrates;
    viper::managers::IMultimediaManagerBase	*multimediaManager;
    dash::mpd::IRepresentation			*representation;

    uint64_t					averageBw;			// Classic EWMA
    uint64_t					instantBw;
    uint64_t					smoothBw;			// Panda paper smoothed y[n]
    uint64_t					targetBw;			// Panda paper x[n] bw estimation
    double					param_Alpha;
    double 					alpha_ewma;
    double					param_Epsilon;
    double					param_K;
    double					param_W;
    double					param_Beta;
    double					param_Bmin;
    double					interTime;				// Actual inter time
    double					targetInterTime;		// Target inter time
    double					downloadTime;

    uint32_t					bufferLevel;
    uint32_t					lastBufferLevel;
    double 					bufferMaxSizeSeconds;		// Usually set to 60s
    double 					bufferLevelSeconds;			// Current buffer level [s]

    double					segmentDuration;
    double					deltaUp;
    double					deltaDown;
    size_t					current;
};

} /* namespace adaptation */
} /* namespace framework */
} /* namespace libdash */

#endif /* LIBDASH_FRAMEWORK_ADAPTATION_PANDA_H_ */
