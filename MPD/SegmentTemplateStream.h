/*
 * SegmentTemplateStream.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_MPD_SEGMENTTEMPLATESTREAM_H_
#define LIBDASH_FRAMEWORK_MPD_SEGMENTTEMPLATESTREAM_H_

#include <math.h>

#include "IMPD.h"
#include "AbstractRepresentationStream.h"
#include "ISegment.h"
#include "ISegmentTemplate.h"

namespace libdash
{
namespace framework
{
namespace mpd
{
class SegmentTemplateStream: public AbstractRepresentationStream
{
public:
    SegmentTemplateStream(dash::mpd::IMPD *mpd, dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet, dash::mpd::IRepresentation *representation);
    virtual ~SegmentTemplateStream();

    virtual dash::mpd::ISegment* getInitializationSegment();
    virtual dash::mpd::ISegment* getIndexSegment(size_t segmentNumber);
    virtual dash::mpd::ISegment* getMediaSegment(size_t segmentNumber);
    virtual dash::mpd::ISegment* getBitstreamSwitchingSegment();
    virtual RepresentationStreamType getStreamType();
    virtual uint32_t getSize();
    virtual uint32_t getAverageSegmentDuration();

private:
    dash::mpd::ISegmentTemplate* findSegmentTemplate();
    void calculateSegmentStartTimes();

    dash::mpd::ISegmentTemplate *segmentTemplate;
    std::vector<uint32_t>       segmentStartTimes;
};
}
}
}

#endif /* LIBDASH_FRAMEWORK_MPD_SEGMENTTEMPLATESTREAM_H_ */
