/*
 * AbstractRepresentationStream.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "AbstractRepresentationStream.h"

using namespace libdash::framework::mpd;
using namespace dash::mpd;

AbstractRepresentationStream::AbstractRepresentationStream(IMPD *mpd, IPeriod *period, IAdaptationSet *adaptationSet, IRepresentation *representation) :
                              mpd                           (mpd),
                              period                        (period),
                              adaptationSet                 (adaptationSet),
                              representation                (representation)
{
}
AbstractRepresentationStream::~AbstractRepresentationStream()
{
}

void AbstractRepresentationStream::setBaseUrls(const std::vector<dash::mpd::IBaseUrl *> baseurls)
{
    this->baseUrls.clear();

    for (size_t i = 0; i < baseurls.size(); i++)
        this->baseUrls.push_back(baseurls.at(i));
}

uint32_t AbstractRepresentationStream::getSize()
{
    return UINT32_MAX - 1;
}

uint32_t AbstractRepresentationStream::getFirstSegmentNumber()
{
    if (this->mpd->GetType() == "dynamic")
    {
        uint32_t currTime = TimeResolver::getCurrentTimeInSec();
        uint32_t availStT = TimeResolver::getUTCDateTimeInSec(this->mpd->GetAvailabilityStarttime());
        uint32_t duration = this->getAverageSegmentDuration();
        uint32_t timeshift = TimeResolver::getDurationInSec(this->mpd->GetTimeShiftBufferDepth());
        return (currTime - duration - availStT - timeshift ) / duration;
    }
    return 0;
}

uint32_t AbstractRepresentationStream::getCurrentSegmentNumber()
{
    if (this->mpd->GetType() == "dynamic")
    {
        uint32_t currTime = TimeResolver::getCurrentTimeInSec();
        uint32_t duration = this->getAverageSegmentDuration();
        uint32_t availStT = TimeResolver::getUTCDateTimeInSec(this->mpd->GetAvailabilityStarttime());

        return (currTime - duration - availStT) / duration;
    }
    return 0;
}

uint32_t AbstractRepresentationStream::getLastSegmentNumber      ()
{
    if (this->mpd->GetType() == "dynamic")
    {
        uint32_t currTime   = TimeResolver::getCurrentTimeInSec();
        uint32_t duration = this->getAverageSegmentDuration();
        uint32_t availStT   = TimeResolver::getUTCDateTimeInSec(this->mpd->GetAvailabilityStarttime());
        uint32_t checkTime  = mpd->GetFetchTime() + 
                              TimeResolver::getDurationInSec(this->mpd->GetMinimumUpdatePeriod());
        return ( ((checkTime > currTime) ? currTime : checkTime) - duration - availStT) / duration;
    }
    return 0;
}

uint32_t AbstractRepresentationStream::getAverageSegmentDuration()
{
    return 1;
}
