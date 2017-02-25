/*
 * AdaptationLogicFactory.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "AdaptationLogicFactory.h"
#include<stdio.h>

using namespace libdash::framework::adaptation;
using namespace dash::mpd;

IAdaptationLogic* AdaptationLogicFactory::create(LogicType logic, IMPD *mpd, IPeriod *period, IAdaptationSet *adaptationSet,bool isVid, struct AdaptationParameters* paramsForAdaptation)
{
    Debug("Adaptation Logic for %s: ", isVid ? "video" : "audio");
    switch(logic)
    {
    case AlwaysLowest:
        Debug("Always lowest\n");
        return new AlwaysLowestLogic(mpd, period, adaptationSet, isVid, paramsForAdaptation);
    case RateBased:
        Debug("Rate based\n");
        return new RateBasedAdaptation(mpd,period,adaptationSet, isVid, paramsForAdaptation);
    case BufferBased:
        Debug("Buffer based\n");
        return new BufferBasedAdaptation(mpd,period,adaptationSet, isVid, paramsForAdaptation);
    case BufferRateBased:
        Debug("Buffer Rate based\n");
        return new BufferBasedAdaptationWithRateBased(mpd,period,adaptationSet, isVid, paramsForAdaptation);
    case BufferBasedThreeThreshold:
        Debug("Buffer based 3 threshold\n");
        return new BufferBasedThreeThresholdAdaptation(mpd,period,adaptationSet, isVid, paramsForAdaptation);
    case Panda:
        Debug("Panda\n");
        return new PandaAdaptation(mpd, period, adaptationSet, isVid, paramsForAdaptation);
    case Bola:
        Debug("Bola\n");
        return new BolaAdaptation(mpd, period, adaptationSet, isVid, paramsForAdaptation);
    default:
        Debug("default => return Always Lowest\n");
        return new AlwaysLowestLogic(mpd, period, adaptationSet, isVid, paramsForAdaptation);
    }
}
