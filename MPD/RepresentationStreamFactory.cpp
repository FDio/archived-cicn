/*
 * RepresentationStreamFactory.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "RepresentationStreamFactory.h"

using namespace libdash::framework::mpd;
using namespace dash::mpd;

IRepresentationStream* RepresentationStreamFactory::create(viper::managers::StreamType streamType, libdash::framework::mpd::RepresentationStreamType type, libdash::framework::mpd::MPDWrapper *mpdWrapper, dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet, dash::mpd::IRepresentation *representation, dash::mpd::IMPD* mpd)
{
    if(mpd)
    {
        switch(type)
        {
            case SingleMediaSegment: return new SingleMediaSegmentStream(streamType, mpdWrapper, period, adaptationSet, representation,mpd);
            case SegmentList:        return new SegmentListStream       (streamType, mpdWrapper, period, adaptationSet, representation,mpd);
            case SegmentTemplate:    return new SegmentTemplateStream   (streamType, mpdWrapper, period, adaptationSet, representation,mpd);

            default:                 return NULL;
       }
    }
    else
    {
        switch(type)
        {
            case SingleMediaSegment: return new SingleMediaSegmentStream(streamType, mpdWrapper, period, adaptationSet, representation);
            case SegmentList:        return new SegmentListStream       (streamType, mpdWrapper, period, adaptationSet, representation);
            case SegmentTemplate:    return new SegmentTemplateStream   (streamType, mpdWrapper, period, adaptationSet, representation);

            default:                 return NULL;
       }
    }
}
