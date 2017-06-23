/*
 * AbstractAdaptationLogic.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "AbstractAdaptationLogic.h"

using namespace libdash::framework::adaptation;
using namespace dash::mpd;

AbstractAdaptationLogic::AbstractAdaptationLogic(viper::managers::StreamType type, libdash::framework::mpd::MPDWrapper *mpdWrapper) :
    mpdWrapper			(mpdWrapper),
    type			(type)
{
        InitializeCriticalSection   (&this->monitorLock);
}

AbstractAdaptationLogic::~AbstractAdaptationLogic()
{
    DeleteCriticalSection   (&this->monitorLock);
}

uint32_t AbstractAdaptationLogic::getPosition()
{
    return 0;
}

void AbstractAdaptationLogic::setPosition(uint32_t segmentNumber)
{
    this->segmentNumber = segmentNumber;
}

IRepresentation* AbstractAdaptationLogic::getRepresentation()
{
//    return this->representation;
}

//void AbstractAdaptationLogic::setRepresentation(IPeriod *period, IAdaptationSet *adaptationSet, IRepresentation *representation)
//{
//    this->period            = period;
//    this->adaptationSet     = adaptationSet;
//    this->representation    = representation;
//}

//void AbstractAdaptationLogic::updateMPD(IMPD* mpd)
//{
//}

void AbstractAdaptationLogic::dLTimeUpdate(double time)
{
}
