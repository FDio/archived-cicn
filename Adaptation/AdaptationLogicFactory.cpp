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
using namespace libdash::framework::mpd;
using namespace dash::mpd;

IAdaptationLogic* AdaptationLogicFactory::create(LogicType logic, viper::managers::StreamType type, MPDWrapper *mpdWrapper, struct AdaptationParameters* paramsForAdaptation)
{
    switch(logic)
    {
    case AlwaysLowest:
        Debug("Always lowest\n");
        return new AlwaysLowestLogic(type, mpdWrapper, paramsForAdaptation);
    case RateBased:
        Debug("Rate based\n");
        return new RateBasedAdaptation(type, mpdWrapper, paramsForAdaptation);
    case BufferBased:
        Debug("Buffer based\n");
        return new BufferBasedAdaptation(type, mpdWrapper, paramsForAdaptation);
    case AdapTech:
        Debug("AdapTech\n");
        return new AdapTechAdaptation(type, mpdWrapper, paramsForAdaptation);
    case BufferBasedThreeThreshold:
        Debug("Buffer based 3 threshold\n");
        return new BufferBasedThreeThresholdAdaptation(type, mpdWrapper, paramsForAdaptation);
    case Panda:
        Debug("Panda\n");
        return new PandaAdaptation(type, mpdWrapper, paramsForAdaptation);
    case Bola:
        Debug("Bola\n");
        return new BolaAdaptation(type, mpdWrapper, paramsForAdaptation);
    default:
        Debug("default => return Always Lowest\n");
        return new AlwaysLowestLogic(type, mpdWrapper, paramsForAdaptation);
    }
}
