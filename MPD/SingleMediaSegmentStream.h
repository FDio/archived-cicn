/*
 * SingleMediaSegmentStream.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_MPD_SINGLEMEDIASEGMENTSTREAM_H_
#define LIBDASH_FRAMEWORK_MPD_SINGLEMEDIASEGMENTSTREAM_H_

#include "IMPD.h"
#include "AbstractRepresentationStream.h"
#include "ISegment.h"

namespace libdash
{
namespace framework
{
namespace mpd
{
class SingleMediaSegmentStream: public AbstractRepresentationStream
{
public:
    SingleMediaSegmentStream(dash::mpd::IMPD *mpd, dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet, dash::mpd::IRepresentation *representation);
    virtual ~SingleMediaSegmentStream();

    virtual dash::mpd::ISegment* getInitializationSegment();
    virtual dash::mpd::ISegment* getIndexSegment(size_t segmentNumber);
    virtual dash::mpd::ISegment* getMediaSegment(size_t segmentNumber);
    virtual dash::mpd::ISegment* getBitstreamSwitchingSegment();
    virtual RepresentationStreamType getStreamType();

    virtual uint32_t getFirstSegmentNumber();
    virtual uint32_t getCurrentSegmentNumber();
    virtual uint32_t getLastSegmentNumber();

};
}
}
}

#endif /* LIBDASH_FRAMEWORK_MPD_SINGLEMEDIASEGMENTSTREAM_H_ */
