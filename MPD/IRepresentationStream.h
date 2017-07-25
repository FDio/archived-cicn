/*
 * IRepresentationStream.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_MPD_IREPRESENTATIONSTREAM_H_
#define LIBDASH_FRAMEWORK_MPD_IREPRESENTATIONSTREAM_H_

#include "ISegment.h"

namespace libdash
{
namespace framework
{
namespace mpd
{
enum RepresentationStreamType
{
    SingleMediaSegment,
    SegmentList,
    SegmentTemplate,
    UNDEFINED
};

class IRepresentationStream
{
public:
    virtual ~IRepresentationStream() {}

    virtual dash::mpd::ISegment* getInitializationSegment() = 0;
    virtual dash::mpd::ISegment* getIndexSegment(size_t segmentNumber) = 0;
    virtual dash::mpd::ISegment* getMediaSegment(size_t segmentNumber, uint64_t& segmentDuration) = 0;
    virtual dash::mpd::ISegment* getBitstreamSwitchingSegment() = 0;
    virtual RepresentationStreamType getStreamType() = 0;
    virtual uint32_t getSize() = 0;
    virtual uint32_t getFirstSegmentNumber() = 0;
    virtual uint32_t getCurrentSegmentNumber() = 0;
    virtual uint32_t getLastSegmentNumber() = 0;
    virtual uint32_t getAverageSegmentDuration() = 0;
    virtual void     setSegmentOffset(uint32_t offset) = 0;
    virtual uint64_t getTime(size_t segmentNumber) = 0;
    virtual size_t getSegmentNumber(uint64_t time) = 0;


};
}
}
}
#endif /* LIBDASH_FRAMEWORK_MPD_IREPRESENTATIONSTREAM_H_ */
