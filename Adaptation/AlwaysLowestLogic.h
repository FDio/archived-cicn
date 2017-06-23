/*
 * AlwaysLowestLogic.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_ADAPTATION_ALWAYSLOWESTLOGIC_H_
#define LIBDASH_FRAMEWORK_ADAPTATION_ALWAYSLOWESTLOGIC_H_

#include "IMPD.h"
#include "AbstractAdaptationLogic.h"

namespace libdash
{
namespace framework
{
namespace adaptation
{
class AlwaysLowestLogic : public AbstractAdaptationLogic
{
public:
//    AlwaysLowestLogic(dash::mpd::IMPD *mpd, dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet, bool isVid, struct AdaptationParameters *params);
    AlwaysLowestLogic(viper::managers::StreamType type, libdash::framework::mpd::MPDWrapper *mpdWrapper, struct AdaptationParameters *params);
    virtual ~AlwaysLowestLogic();

    virtual LogicType getType();
    virtual bool isUserDependent();
    virtual bool isRateBased();
    virtual bool isBufferBased();
    virtual void bitrateUpdate(uint64_t, uint32_t);
    virtual void dLTimeUpdate(double time);
    virtual void bufferUpdate(uint32_t, int);
    virtual void setMultimediaManager(viper::managers::IMultimediaManagerBase *mmM);
    virtual void onEOS(bool value);
    virtual void checkedByDASHReceiver();
private:


};
}
}
}

#endif /* LIBDASH_FRAMEWORK_ADAPTATION_ALWAYSLOWESTLOGIC_H_ */
