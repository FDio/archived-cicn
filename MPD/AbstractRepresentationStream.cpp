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

AbstractRepresentationStream::AbstractRepresentationStream(viper::managers::StreamType type, IMPDWrapper *mpdWrapper, IPeriod *period, IAdaptationSet *adaptationSet, IRepresentation *representation) :
                              type			    (type),
			      mpdWrapper                    (mpdWrapper),
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
    if (this->mpdWrapper->getTypeWithoutLock() == "dynamic")
    {
        uint32_t currTime = TimeResolver::getCurrentTimeInSec();
        uint32_t availStT = TimeResolver::getUTCDateTimeInSec(this->mpdWrapper->getAvailabilityStarttime());
        uint32_t duration = this->getAverageSegmentDuration();
        uint32_t timeshift = TimeResolver::getDurationInSec(this->mpdWrapper->getTimeShiftBufferDepth());
        uint32_t timescale = this->getTimescale();
        return (double)((double)currTime - (double)availStT - (double)timeshift ) < 0? 0 : (currTime - availStT - timeshift );
    }
    return 0;
}

uint32_t AbstractRepresentationStream::getCurrentSegmentNumber()
{
    if (this->mpdWrapper->getTypeWithoutLock() == "dynamic")
    {
        uint32_t currTime = TimeResolver::getCurrentTimeInSec();
        uint32_t duration = this->getAverageSegmentDuration();
        uint32_t availStT = TimeResolver::getUTCDateTimeInSec(this->mpdWrapper->getAvailabilityStarttime());

        return (double)((double)currTime - (double)availStT) < 0 ? 0 : (currTime - availStT);
    //    return (currTime - duration - availStT) / duration;
    }
    return 0;
}

uint32_t AbstractRepresentationStream::getLastSegmentNumber      ()
{
    if (this->mpdWrapper->getTypeWithoutLock() == "dynamic")
    {
        uint32_t currTime   = TimeResolver::getCurrentTimeInSec();
        uint32_t duration = this->getAverageSegmentDuration();
        uint32_t availStT   = TimeResolver::getUTCDateTimeInSec(this->mpdWrapper->getAvailabilityStarttime());
        uint32_t checkTime  = mpdWrapper->getFetchTime() + 
                              TimeResolver::getDurationInSec(this->mpdWrapper->getMinimumUpdatePeriodWithoutLock());
        return ( ((checkTime > currTime) ? currTime : checkTime) - duration - availStT) / duration;
    }
    return 0;
}

uint32_t AbstractRepresentationStream::getAverageSegmentDuration()
{
    return 1;
}
uint32_t    AbstractRepresentationStream::getTimescale ()
{
    return 1;
}
void	    AbstractRepresentationStream::setSegmentOffset (uint32_t offset)
{
    this->segmentOffset = offset;
}

uint32_t AbstractRepresentationStream::getTime(size_t segmentNumber)
{
    return 0;
}
size_t AbstractRepresentationStream::getSegmentNumber(uint32_t time)
{
    return 0;
}
