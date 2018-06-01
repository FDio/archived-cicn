/*
 * SingleMediaSegmentStream.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "SingleMediaSegmentStream.h"

using namespace dash::mpd;
using namespace libdash::framework::mpd;

SingleMediaSegmentStream::SingleMediaSegmentStream(viper::managers::StreamType type, MPDWrapper *mpdWrapper, IPeriod *period, IAdaptationSet *adaptationSet, IRepresentation *representation) :
                          AbstractRepresentationStream  (type, mpdWrapper, period, adaptationSet, representation)
{
    this->baseUrls = BaseUrlResolver::resolveBaseUrl(type, mpdWrapper, 0, 0, 0);
}

SingleMediaSegmentStream::SingleMediaSegmentStream(viper::managers::StreamType type, MPDWrapper *mpdWrapper, IPeriod *period, IAdaptationSet *adaptationSet, IRepresentation *representation, IMPD* mpd) :
                          AbstractRepresentationStream  (type, mpdWrapper, period, adaptationSet, representation)
{
    this->baseUrls = BaseUrlResolver::resolveBaseUrl(type, mpdWrapper, 0, 0, 0, mpd);
}
SingleMediaSegmentStream::~SingleMediaSegmentStream()
{
}
ISegment* SingleMediaSegmentStream::getInitializationSegment()
{
    if (this->representation->GetSegmentBase())
    {
        /* TODO: check whether @sourceURL and/or @range is available in SegmentBase.Initialization, see Table 13 */

        if (this->representation->GetSegmentBase()->GetInitialization())
            return this->representation->GetSegmentBase()->GetInitialization()->ToSegment(baseUrls);
    }

    return NULL;
}
ISegment* SingleMediaSegmentStream::getIndexSegment(size_t segmentNumber)
{
    /* segmentNumber is not used in this case */
    if (this->representation->GetSegmentBase())
    {
        if (this->representation->GetSegmentBase()->GetRepresentationIndex())
            return this->representation->GetSegmentBase()->GetRepresentationIndex()->ToSegment(baseUrls);
    }

    return NULL;
}
ISegment* SingleMediaSegmentStream::getMediaSegment(size_t segmentNumber)
{
    /* segmentNumber equals the desired BaseUrl */
    if (this->representation->GetBaseURLs().size() > segmentNumber)
        return this->representation->GetBaseURLs().at(segmentNumber)->ToMediaSegment(baseUrls);

    return this->representation->GetBaseURLs().at(0)->ToMediaSegment(baseUrls);
}
ISegment* SingleMediaSegmentStream::getBitstreamSwitchingSegment()
{
    /* not possible */
    return NULL;
}
RepresentationStreamType SingleMediaSegmentStream::getStreamType()
{
    return SingleMediaSegment;
}
uint32_t SingleMediaSegmentStream::getFirstSegmentNumber()
{
    return 0;
}
uint32_t SingleMediaSegmentStream::getCurrentSegmentNumber()
{
    return 0;
}
uint32_t SingleMediaSegmentStream::getLastSegmentNumber()
{
    return 0;
}
