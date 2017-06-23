/*
 * AbstractAdaptationLogic.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_ADAPTATION_ABSTRACTADAPTATIONLOGIC_H_
#define LIBDASH_FRAMEWORK_ADAPTATION_ABSTRACTADAPTATIONLOGIC_H_

#include "IAdaptationLogic.h"
#include "IMPD.h"

namespace libdash
{
namespace framework
{
namespace adaptation
{
class AbstractAdaptationLogic : public IAdaptationLogic
{
public:
//    AbstractAdaptationLogic(dash::mpd::IMPD *mpd, dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet *adaptationSet, bool isVideo);
    AbstractAdaptationLogic(viper::managers::StreamType type, libdash::framework::mpd::MPDWrapper *mpdWrapper);
    virtual ~AbstractAdaptationLogic();

    virtual uint32_t getPosition();
    virtual void setPosition(uint32_t segmentNumber);
    virtual dash::mpd::IRepresentation* getRepresentation   ();
//    virtual void setRepresentation(dash::mpd::IPeriod *period,
//                                   dash::mpd::IAdaptationSet *adaptationSet,
//                                   dash::mpd::IRepresentation *representation);
//    virtual void updateMPD(dash::mpd::IMPD* mpd);

    virtual LogicType getType() 			= 0;
    virtual bool isUserDependent() 			= 0;
    virtual bool isRateBased()				= 0;
    virtual bool isBufferBased()			= 0;
    virtual void bitrateUpdate(uint64_t, uint32_t)	= 0;
    virtual void bufferUpdate(uint32_t, int)		= 0;
    virtual void onEOS(bool value)			= 0;
    virtual void dLTimeUpdate(double)			= 0;
    virtual void checkedByDASHReceiver()		= 0;

protected:
    libdash::framework::mpd::MPDWrapper	*mpdWrapper;
//    dash::mpd::IPeriod                  *period;
//    dash::mpd::IAdaptationSet           *adaptationSet;
//    dash::mpd::IRepresentation          *representation;
    uint32_t                            segmentNumber;
//    bool				isVideo;
    viper::managers::StreamType		type;
    mutable CRITICAL_SECTION        	monitorLock;
};
}
}
}
#endif /* LIBDASH_FRAMEWORK_ADAPTATION_ABSTRACTADAPTATIONLOGIC_H_ */
