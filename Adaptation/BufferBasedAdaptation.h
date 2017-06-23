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

#ifndef LIBDASH_FRAMEWORK_ADAPTATION_BUFFERBASEDADAPTATION_H_
#define LIBDASH_FRAMEWORK_ADAPTATION_BUFFERBASEDADAPTATION_H_

#include "AbstractAdaptationLogic.h"
#include "../MPD/AdaptationSetStream.h"
#include "../Input/IDASHReceiverObserver.h"

namespace libdash
{
namespace framework
{
namespace adaptation
{
class BufferBasedAdaptation : public AbstractAdaptationLogic
{
public:
//    BufferBasedAdaptation(dash::mpd::IMPD *mpd, dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet, bool isVid, struct AdaptationParameters *params);
    BufferBasedAdaptation(viper::managers::StreamType type, libdash::framework::mpd::MPDWrapper *mpdWrapper, struct AdaptationParameters *params);
    virtual ~BufferBasedAdaptation();

    virtual LogicType getType();
    virtual bool isUserDependent();
    virtual bool isRateBased();
    virtual bool isBufferBased();
    virtual void bitrateUpdate(uint64_t bps, uint32_t segNum);
    virtual void bufferUpdate(uint32_t bufferFill, int maxC);
    virtual void dLTimeUpdate(double time);
    void setBitrate(uint32_t bufferFill);
    uint64_t getBitrate();
    virtual void setMultimediaManager(viper::managers::IMultimediaManagerBase *_mmManager);
    void notifyBitrateChange();
    void onEOS(bool value);
    void checkedByDASHReceiver();

private:
    uint64_t					currentBitrate;
    std::vector<uint64_t>			availableBitrates;
    viper::managers::IMultimediaManagerBase	*multimediaManager;
    dash::mpd::IRepresentation			*representation;
    uint32_t					reservoirThreshold;
    uint32_t					maxThreshold;
    uint32_t					lastBufferFill;
    bool					bufferEOS;
    bool					shouldAbort;
};
}
}
}

#endif /* LIBDASH_FRAMEWORK_ADAPTATION_BUFFERBASEDADAPTATION_H_ */
