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

#include "IRepresentationStream.h"
#include "IBaseUrl.h"
#include "IRepresentation.h"
#include "IAdaptationSet.h"
#include "IMPD.h"
#include "IPeriod.h"
#include "BaseUrlResolver.h"
#include "TimeResolver.h"

namespace libdash
{
namespace framework
{
namespace mpd
{
class AbstractRepresentationStream : public IRepresentationStream
{
public:
    AbstractRepresentationStream(dash::mpd::IMPD *mpd, dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet,
                                 dash::mpd::IRepresentation *representation);
    virtual ~AbstractRepresentationStream();

    virtual dash::mpd::ISegment* getInitializationSegment() = 0;
    virtual dash::mpd::ISegment* getIndexSegment(size_t segmentNumber) = 0;
    virtual dash::mpd::ISegment* getMediaSegment(size_t segmentNumber) = 0;
    virtual dash::mpd::ISegment* getBitstreamSwitchingSegment() = 0;
    virtual RepresentationStreamType getStreamType() = 0;

    virtual uint32_t getSize();
    virtual uint32_t getFirstSegmentNumber();
    virtual uint32_t getCurrentSegmentNumber();
    virtual uint32_t getLastSegmentNumber();
    virtual uint32_t getAverageSegmentDuration();

protected:
    virtual void setBaseUrls(const std::vector<dash::mpd::IBaseUrl *> baseurls);

    std::vector<dash::mpd::IBaseUrl *>  baseUrls;
    dash::mpd::IMPD                     *mpd;
    dash::mpd::IPeriod                  *period;
    dash::mpd::IAdaptationSet           *adaptationSet;
    dash::mpd::IRepresentation          *representation;
};
}
}
}
#endif /* LIBDASH_FRAMEWORK_ADAPTATION_ABSTRACTADAPTATIONLOGIC_H_ */
