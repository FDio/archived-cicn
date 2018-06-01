/*
 * SegmentListStream.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_MPD_SEGMENTLISTSTREAM_H_
#define LIBDASH_FRAMEWORK_MPD_SEGMENTLISTSTREAM_H_

#include "MPDWrapper.h"
#include "AbstractRepresentationStream.h"
#include "ISegment.h"
#include "ISegmentList.h"

namespace libdash
{
namespace framework
{
namespace mpd
{
class SegmentListStream: public AbstractRepresentationStream
{
public:
    SegmentListStream(viper::managers::StreamType type, MPDWrapper *mpdWrapper, dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet, dash::mpd::IRepresentation *representation);
    SegmentListStream(viper::managers::StreamType type, MPDWrapper *mpdWrapper, dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet, dash::mpd::IRepresentation *representation, dash::mpd::IMPD* mpd);
//    SegmentListStream(viper::managers::StreamType type, dash::mpd::MPDWrapper *mpdWrapper);
    virtual ~SegmentListStream();

    virtual dash::mpd::ISegment* getInitializationSegment();
    virtual dash::mpd::ISegment* getIndexSegment(size_t segmentNumber);
    virtual dash::mpd::ISegment* getMediaSegment(size_t segmentNumber);
    virtual dash::mpd::ISegment* getBitstreamSwitchingSegment();
    virtual RepresentationStreamType getStreamType();
    virtual uint32_t getSize();
    virtual uint32_t getAverageSegmentDuration();
    virtual uint32_t getTimescale();

private:
    dash::mpd::ISegmentList  *findSegmentList();

    dash::mpd::ISegmentList  *segmentList;

};
}
}
}

#endif /* LIBDASH_FRAMEWORK_MPD_SEGMENTLISTSTREAM_H_ */
