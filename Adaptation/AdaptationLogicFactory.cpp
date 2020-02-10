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
        qDebug("Always lowest");
        return new AlwaysLowestLogic(type, mpdWrapper, paramsForAdaptation);
    case RateBased:
        qDebug("Rate based");
        return new RateBasedAdaptation(type, mpdWrapper, paramsForAdaptation);
    case BufferBased:
        qDebug("Buffer based");
        return new BufferBasedAdaptation(type, mpdWrapper, paramsForAdaptation);
    case AdapTech:
        qDebug("AdapTech");
        return new AdapTechAdaptation(type, mpdWrapper, paramsForAdaptation);
    case BufferBasedThreeThreshold:
        qDebug("Buffer based 3 threshold");
        return new BufferBasedThreeThresholdAdaptation(type, mpdWrapper, paramsForAdaptation);
    case Panda:
        qDebug("Panda");
        return new PandaAdaptation(type, mpdWrapper, paramsForAdaptation);
    case Bola:
        qDebug("Bola");
        return new BolaAdaptation(type, mpdWrapper, paramsForAdaptation);
    default:
        qDebug("default => return Always Lowest");
        return new AlwaysLowestLogic(type, mpdWrapper, paramsForAdaptation);
    }
}
