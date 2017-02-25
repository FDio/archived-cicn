/*
 * AdaptationSetHelper.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "AdaptationSetHelper.h"

using namespace libdash::framework::mpd;
using namespace dash::mpd;

std::vector<IAdaptationSet *> AdaptationSetHelper::getAudioAdaptationSets (dash::mpd::IPeriod *period)
{
    std::vector<IAdaptationSet *> audioAdaptationSets;
    std::vector<IAdaptationSet *> adaptationSets = period->GetAdaptationSets();
    for (size_t i = 0; i < adaptationSets.size(); i++)
        if (AdaptationSetHelper::isAudioAdaptationSet(adaptationSets.at(i)))
            audioAdaptationSets.push_back(adaptationSets.at(i));

    return audioAdaptationSets;
}

std::vector<IAdaptationSet *> AdaptationSetHelper::getVideoAdaptationSets (dash::mpd::IPeriod *period)
{
    std::vector<IAdaptationSet *> videoAdaptationSets;
    std::vector<IAdaptationSet *> adaptationSets = period->GetAdaptationSets();
    for (size_t i = 0; i < adaptationSets.size(); i++) {
        if (AdaptationSetHelper::isVideoAdaptationSet(adaptationSets.at(i))) {
            videoAdaptationSets.push_back(adaptationSets.at(i));
        }
    }
    return videoAdaptationSets;
}

bool AdaptationSetHelper::isAudioAdaptationSet(dash::mpd::IAdaptationSet *adaptationSet)
{
    return isContainedInMimeType(adaptationSet, "audio");
}

bool AdaptationSetHelper::isVideoAdaptationSet(dash::mpd::IAdaptationSet *adaptationSet)
{
    return isContainedInMimeType(adaptationSet, "video");
}

bool AdaptationSetHelper::isContainedInMimeType  (dash::mpd::IAdaptationSet *adaptationSet, std::string value)
{
    for (size_t i = 0; i < adaptationSet->GetRepresentation().size(); i++)
        if (adaptationSet->GetRepresentation().at(i)->GetMimeType() != "")
            if (adaptationSet->GetRepresentation().at(i)->GetMimeType().find(value) != std::string::npos)
                return true;

    return false;
}
