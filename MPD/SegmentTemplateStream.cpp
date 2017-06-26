/*
 * SegmentTemplateStream.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "SegmentTemplateStream.h"

using namespace dash::mpd;
using namespace libdash::framework::mpd;

SegmentTemplateStream::SegmentTemplateStream(viper::managers::StreamType type, MPDWrapper *mpdWrapper, IPeriod *period, IAdaptationSet *adaptationSet, IRepresentation *representation) :
    AbstractRepresentationStream     (type, mpdWrapper, period, adaptationSet, representation)
{
//    this->baseUrls          = BaseUrlResolver::resolveBaseUrl(mpd, period, adaptationSet, 0, 0, 0);
    this->baseUrls          = BaseUrlResolver::resolveBaseUrl(type, mpdWrapper, 0, 0, 0);
    this->segmentTemplate   = findSegmentTemplate();
    this->inSync = false;
    this->currentSegment = 0;
    calculateSegmentStartTimes();
}

SegmentTemplateStream::SegmentTemplateStream(viper::managers::StreamType type, MPDWrapper *mpdWrapper, IPeriod *period, IAdaptationSet *adaptationSet, IRepresentation *representation, IMPD* mpd) :
    AbstractRepresentationStream     (type, mpdWrapper, period, adaptationSet, representation)
{
//    this->baseUrls          = BaseUrlResolver::resolveBaseUrl(mpd, period, adaptationSet, 0, 0, 0);
    this->baseUrls          = BaseUrlResolver::resolveBaseUrl(type, mpdWrapper, 0, 0, 0, mpd);
    this->segmentTemplate   = findSegmentTemplate();
    this->inSync = false;
    this->currentSegment = 0;
    calculateSegmentStartTimes();
}

SegmentTemplateStream::~SegmentTemplateStream()
{
}

ISegment* SegmentTemplateStream::getInitializationSegment()
{
    if (this->segmentTemplate->GetInitialization())
        return this->segmentTemplate->GetInitialization()->ToSegment(baseUrls);

    return this->segmentTemplate->ToInitializationSegment(baseUrls, representation->GetId(), representation->GetBandwidth());
}

ISegment* SegmentTemplateStream::getIndexSegment(size_t segmentNumber)
{
    /* time-based template */
    if (this->segmentTemplate->GetSegmentTimeline())
    {
        if (this->segmentStartTimes.size() > segmentNumber)
            return this->segmentTemplate->GetIndexSegmentFromTime(baseUrls, representation->GetId(), representation->GetBandwidth(), this->segmentStartTimes.at(segmentNumber));

        return NULL;
    }

    /* number-based template */
    return this->segmentTemplate->GetIndexSegmentFromNumber(baseUrls, representation->GetId(), representation->GetBandwidth(),
                                                            this->segmentTemplate->GetStartNumber() + segmentNumber);
}

ISegment* SegmentTemplateStream::getMediaSegment(size_t segmentNumber)
{
    /* time-based template */
    if (this->segmentTemplate->GetSegmentTimeline())
    {//Get the one at segmentNumber
    	if(this->segmentStartTimes.size() > segmentNumber)
	    return this->segmentTemplate->GetMediaSegmentFromTime(baseUrls, representation->GetId(), representation->GetBandwidth(), this->segmentStartTimes.at(segmentNumber));
    	else
	    return NULL;
	
//The following is to be used if you wish to start directly from the right time
/*  {
        if(this->inSync)
        {
            this->currentSegment++;
            if(this->currentSegment < this->segmentStartTimes.size())
                return this->segmentTemplate->GetMediaSegmentFromTime(baseUrls, representation->GetId(), representation->GetBandwidth(), this->segmentStartTimes.at(this->currentSegment));
            else
                return NULL;
        }

        //Look for the start point of segment, ie: the closest lowest to segmentNumber
        //adding segment offset
        segmentNumber = segmentNumber + this->segmentOffset;

        size_t prevSegNumber = 0;
        size_t segNumber = 0;
        for(segNumber = 0; segNumber < this->segmentStartTimes.size(); segNumber++)
        {
            if(this->segmentStartTimes.at(segNumber)/this->getTimescale() > segmentNumber)
            {
                segNumber = prevSegNumber;
                break;
            }
            prevSegNumber = segNumber;
        }
        if(segNumber != this->segmentStartTimes.size())
        {
            this->inSync = true;
            this->currentSegment = segNumber;
            return this->segmentTemplate->GetMediaSegmentFromTime(baseUrls, representation->GetId(), representation->GetBandwidth(), this->segmentStartTimes.at(this->currentSegment));
        }
        return NULL; */
    }

    /* number-based template */
    return this->segmentTemplate->GetMediaSegmentFromNumber(baseUrls, representation->GetId(), representation->GetBandwidth(),
                                                            this->segmentTemplate->GetStartNumber() + segmentNumber);

}

ISegment* SegmentTemplateStream::getBitstreamSwitchingSegment ()
{
    if (this->segmentTemplate->GetBitstreamSwitching())
        return this->segmentTemplate->GetBitstreamSwitching()->ToSegment(baseUrls);
    return this->segmentTemplate->ToBitstreamSwitchingSegment(baseUrls, representation->GetId(), representation->GetBandwidth());
}

RepresentationStreamType SegmentTemplateStream::getStreamType()
{
    return SegmentList;
}

uint32_t SegmentTemplateStream::getSize()
{
    if (!this->segmentStartTimes.empty())
        return this->segmentStartTimes.size();

    uint32_t numberOfSegments          = 0;
    double   mediaPresentationDuration = 0;

    if (this->mpdWrapper->getTypeWithoutLock() == "static")
    {
        mediaPresentationDuration = TimeResolver::getDurationInSec(this->mpdWrapper->getMediaPresentationDuration());
        numberOfSegments = (uint32_t) ceil(mediaPresentationDuration / (this->segmentTemplate->GetDuration() / this->segmentTemplate->GetTimescale()));
    }
    else
    {
        numberOfSegments = UINT32_MAX - 1;
    }
    return numberOfSegments;
}

uint32_t		    SegmentTemplateStream::getTimescale			()
{
    return this->segmentTemplate->GetTimescale();
}

uint32_t SegmentTemplateStream::getAverageSegmentDuration()
{
    /* TODO calculate average segment durations for SegmentTimeline */
    return this->averageDuration;
//    return this->segmentTemplate->GetDuration();
}

ISegmentTemplate* SegmentTemplateStream::findSegmentTemplate()
{
    if (this->representation->GetSegmentTemplate())
        return this->representation->GetSegmentTemplate();

    if (this->adaptationSet->GetSegmentTemplate())
        return this->adaptationSet->GetSegmentTemplate();

    if (this->period->GetSegmentTemplate())
        return this->period->GetSegmentTemplate();

    return NULL;
}

void SegmentTemplateStream::calculateSegmentStartTimes()
{
    if (!this->segmentTemplate->GetSegmentTimeline())
        return;

    size_t   numOfTimelines = 0;
    uint64_t segStartTime   = 0;
    uint64_t segDuration    = 0;
    size_t   repeatCount    = 0;
    uint64_t totalDuration  = 0;

    numOfTimelines      = this->segmentTemplate->GetSegmentTimeline()->GetTimelines().size();

    for (size_t i = 0; i < numOfTimelines; i++)
    {
        repeatCount     = this->segmentTemplate->GetSegmentTimeline()->GetTimelines().at(i)->GetRepeatCount();
        segStartTime    = this->segmentTemplate->GetSegmentTimeline()->GetTimelines().at(i)->GetStartTime();
        segDuration     = this->segmentTemplate->GetSegmentTimeline()->GetTimelines().at(i)->GetDuration();
        totalDuration   = totalDuration + segDuration;
        if (repeatCount > 0)
        {
            for (size_t j = 0; j <= repeatCount; j++)
            {
                if (segStartTime > 0)
                {
                    this->segmentStartTimes.push_back(segStartTime + segDuration * j);
                }
                else
                {
                    /* TODO: implement if SegmentTemplate.SegmentTimeline.S@t is not specified */
                }
            }
        }
        else
        {
            this->segmentStartTimes.push_back(segStartTime);
        }
    }
    this->averageDuration = totalDuration / numOfTimelines;
}

uint64_t SegmentTemplateStream::getTime(size_t segmentNumber)
{
    if(segmentNumber < this->segmentStartTimes.size())
	return this->segmentStartTimes.at(segmentNumber);
    else
	return 0;
}

size_t SegmentTemplateStream::getSegmentNumber(uint64_t time)
{
    size_t i;
    for(i = 0; i < this->segmentStartTimes.size(); i ++)
    {
	if(time <= this->segmentStartTimes.at(i))
	{
	    break;
	}
    }
    return i;
}
