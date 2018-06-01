/*
 * AbstracRepresenationStream.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_MPD_ABSTRACTREPRESENTATIONSTREAM_H_
#define LIBDASH_FRAMEWORK_MPD_ABSTRACTREPRESENTATIONSTREAM_H_

#include "../MPD/IMPDWrapper.h"
#include "IRepresentationStream.h"
#include "IBaseUrl.h"
#include "IRepresentation.h"
#include "IAdaptationSet.h"
#include "IPeriod.h"
#include "BaseUrlResolver.h"
#include "TimeResolver.h"

namespace viper
{
namespace managers
{
enum StreamType;
}
}
namespace libdash
{
namespace framework
{
namespace mpd
{
class AbstractRepresentationStream : public IRepresentationStream
{
public:
    AbstractRepresentationStream(viper::managers::StreamType type, libdash::framework::mpd::IMPDWrapper *mpdWrapper, dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet,
                                 dash::mpd::IRepresentation *representation);
    virtual ~AbstractRepresentationStream();

    virtual dash::mpd::ISegment* getInitializationSegment() = 0;
    virtual dash::mpd::ISegment* getIndexSegment(size_t segmentNumber) = 0;
    virtual dash::mpd::ISegment* getMediaSegment(size_t segmentNumber) = 0;
    virtual dash::mpd::ISegment* getBitstreamSwitchingSegment() = 0;
    virtual RepresentationStreamType getStreamType() = 0;

    virtual uint32_t    getSize();
    virtual uint32_t    getFirstSegmentNumber();
    virtual uint32_t    getCurrentSegmentNumber();
    virtual uint32_t    getLastSegmentNumber();
    virtual uint32_t    getAverageSegmentDuration();

    virtual uint32_t    getTimescale();
    virtual void        setSegmentOffset(uint32_t offset);
    virtual uint64_t    getTime(size_t segmentNumber);
    virtual size_t      getSegmentNumber(uint64_t time);

protected:
    virtual void setBaseUrls(const std::vector<dash::mpd::IBaseUrl *> baseurls);

    std::vector<dash::mpd::IBaseUrl *>      baseUrls;
    libdash::framework::mpd::IMPDWrapper    *mpdWrapper;
    dash::mpd::IPeriod                      *period;
    dash::mpd::IAdaptationSet               *adaptationSet;
    dash::mpd::IRepresentation              *representation;
    uint32_t                                segmentOffset;
    viper::managers::StreamType             type;
};
}
}
}
#endif /* LIBDASH_FRAMEWORK_ADAPTATION_ABSTRACTADAPTATIONLOGIC_H_ */
