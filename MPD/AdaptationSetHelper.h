/*
 * AdaptationSetHelper.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_MPD_ADAPTATIONSETHELPER_H_
#define LIBDASH_FRAMEWORK_MPD_ADAPTATIONSETHELPER_H_

#include "IMPD.h"

namespace libdash
{
namespace framework
{
namespace mpd
{
class AdaptationSetHelper
{
public:
    static std::vector<dash::mpd::IAdaptationSet *> getAudioAdaptationSets(dash::mpd::IPeriod *period);
    static std::vector<dash::mpd::IAdaptationSet *> getVideoAdaptationSets(dash::mpd::IPeriod *period);
    static bool isAudioAdaptationSet(dash::mpd::IAdaptationSet *adaptationSet);
    static bool isVideoAdaptationSet(dash::mpd::IAdaptationSet *adaptationSet);
    static bool isContainedInMimeType(dash::mpd::IAdaptationSet *adaptationSet, std::string value);
};
}
}
}
#endif /* LIBDASH_FRAMEWORK_MPD_ADAPTATIONSETHELPER_H_ */
