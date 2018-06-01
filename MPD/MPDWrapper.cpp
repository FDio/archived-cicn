/*
 * MPDWrapper.cpp
 *****************************************************************************
 * Copyright (C) 2017, Cisco Systems France
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "MPDWrapper.h"

using namespace dash::mpd;
using namespace libdash::framework::mpd;
using namespace libdash::framework::input;
using namespace viper::managers;

MPDWrapper::MPDWrapper(IMPD *mpd):
    mpd			(mpd),
    period		(NULL),
    videoAdaptationSet	(NULL),
    videoRepresentation	(NULL),
    audioAdaptationSet	(NULL),
    audioRepresentation	(NULL),
    videoRepresentations(NULL),
    audioRepresentations(NULL),
    videoSegmentOffset	(0),
    audioSegmentOffset	(0),
    videoSegmentNumber	(0),
    audioSegmentNumber	(0)

{
    InitializeConditionVariable (&this->mpdUpdate);
    InitializeCriticalSection(&this->monitorMutex);
}

MPDWrapper::~MPDWrapper()
{
    DeleteCriticalSection(&this->monitorMutex);
    DeleteConditionVariable(&this->mpdUpdate);
}

IMPD*	MPDWrapper::getMPD	()
{
    return this->mpd;
}

void	MPDWrapper::updateMPD	(IMPD* mpd)
{
//Assumptions here:
//    *only one period in the MPD
//    *only triggered if using SegmentTimeline dynamic MPD
    this->acquireLock();
    this->period = mpd->GetPeriods().at(0);
    this->findVideoAdaptationSet(mpd);
    this->findAudioAdaptationSet(mpd);
    this->findVideoRepresentation(mpd);
    this->findAudioRepresentation(mpd);
    delete(this->mpd);
    this->mpd = mpd;
    WakeAllConditionVariable(&this->mpdUpdate);
    this->releaseLock();
}

void	MPDWrapper::findVideoAdaptationSet	(IMPD* mpd)
{
    std::vector<IAdaptationSet *> adaptationSets = AdaptationSetHelper::getVideoAdaptationSets(mpd->GetPeriods().at(0));
    if(!(adaptationSets.empty()) && this->videoAdaptationSet)
    {
        for(size_t i = 0; i < adaptationSets.size(); i++)
        {
            if(adaptationSets.at(i)->GetId() == this->videoAdaptationSet->GetId())
            {
                this->videoAdaptationSet = adaptationSets.at(i);
                return;
            }
        }
        //Not found in the new set of adaptation logc => select the first one
        this->videoAdaptationSet = adaptationSets.at(0);
    }
    else
    {
        if(!adaptationSets.empty())
        {
            this->videoAdaptationSet = adaptationSets.at(0);
            return;
        }
        this->videoAdaptationSet = NULL;
    }
}

void	MPDWrapper::findAudioAdaptationSet	(IMPD* mpd)
{
    std::vector<IAdaptationSet *> adaptationSets = AdaptationSetHelper::getAudioAdaptationSets(mpd->GetPeriods().at(0));
    if(!(adaptationSets.empty()) && this->audioAdaptationSet)
    {
        for(size_t i = 0; i < adaptationSets.size(); i++)
        {
            if(adaptationSets.at(i)->GetId() == this->audioAdaptationSet->GetId())
            {
                this->audioAdaptationSet = adaptationSets.at(i);
                return;
            }
        }
        //Not found in the new set of adaptation logc => select the first one
        this->audioAdaptationSet = adaptationSets.at(0);

    }
    else
    {
        if(!adaptationSets.empty())
        {
            this->audioAdaptationSet = adaptationSets.at(0);
            return;
        }
        this->audioAdaptationSet = NULL;
    }
}

void	MPDWrapper::findVideoRepresentation	(IMPD* mpd)
{
    if(this->videoAdaptationSet)
    {
        std::vector<IRepresentation *> representations = this->videoAdaptationSet->GetRepresentation();
        if(this->videoRepresentation)
        {
            uint32_t time = this->videoRepresentations->find(this->videoRepresentation)->second->getTime(this->videoSegmentNumber);
            uint32_t id = atoi(this->videoRepresentation->GetId().c_str());
            for(size_t i = 0; i < representations.size(); i++)
            {
                if(id == atoi(representations.at(i)->GetId().c_str()))
                {
                    this->videoRepresentation = representations.at(i);
                    this->destroyAdaptationSetStream(viper::managers::StreamType::VIDEO);
                    this->initializeAdaptationSetStreamWithoutLock(viper::managers::StreamType::VIDEO, mpd);
                    this->videoSegmentNumber = this->videoRepresentations->find(this->videoRepresentation)->second->getSegmentNumber(time);
                    return;
                }
            }
            this->destroyAdaptationSetStream(viper::managers::StreamType::VIDEO);
        }
        this->videoRepresentation = representations.at(0);
        this->initializeAdaptationSetStreamWithoutLock(viper::managers::StreamType::VIDEO, mpd);
        this->videoSegmentNumber = 0;
    }
    else
    {
        this->videoRepresentation = NULL;
    }
}

void	MPDWrapper::findAudioRepresentation	(IMPD* mpd)
{
    if(this->audioAdaptationSet)
    {
        std::vector<IRepresentation *> representations = this->audioAdaptationSet->GetRepresentation();
        if(this->audioRepresentation)
        {
            uint32_t time = this->audioRepresentations->find(this->audioRepresentation)->second->getTime(this->audioSegmentNumber);
            uint32_t id = atoi(this->audioRepresentation->GetId().c_str());
            for(size_t i = 0; i < representations.size(); i++)
            {
                if(id == atoi(representations.at(i)->GetId().c_str()))
                {
                    this->audioRepresentation = representations.at(i);
                    this->destroyAdaptationSetStream(viper::managers::StreamType::AUDIO);
                    this->initializeAdaptationSetStreamWithoutLock(viper::managers::StreamType::AUDIO, mpd);
                    this->audioSegmentNumber = this->audioRepresentations->find(this->audioRepresentation)->second->getSegmentNumber(time);
                    return;
                }
            }
            this->destroyAdaptationSetStream(viper::managers::StreamType::AUDIO);
        }
        this->audioRepresentation = representations.at(0);
        this->initializeAdaptationSetStreamWithoutLock(viper::managers::StreamType::AUDIO,mpd);
        this->audioSegmentNumber = 0;
    }
    else
    {
        this->audioRepresentation = NULL;
    }
}

std::string	MPDWrapper::getType	()
{
    std::string type;
    this->acquireLock();
    type = this->mpd->GetType();
    this->releaseLock();
    return type;
}


void	MPDWrapper::reInit	(viper::managers::StreamType type)
{
    this->acquireLock();
    switch(type)
    {
    case viper::managers::StreamType::VIDEO:
    {
        this->period = NULL;
        this->videoAdaptationSet = NULL;
        this->videoRepresentation = NULL;
        break;
    }
    case viper::managers::StreamType::AUDIO:
    {
        this->period = NULL;
        this->audioAdaptationSet = NULL;
        this->audioRepresentation = NULL;
        break;
    }
    default:
        break;
    }
    this->releaseLock();
}

bool	MPDWrapper::hasVideoAdaptationSetAndVideoRepresentation	()
{
    this->acquireLock();
    if(this->videoAdaptationSet && this->videoRepresentation)
    {
        this->releaseLock();
        return 1;
    }
    this->releaseLock();
    return 0;
}

bool	MPDWrapper::hasAudioAdaptationSetAndAudioRepresentation	()
{
    this->acquireLock();
    if(this->audioAdaptationSet && this->audioRepresentation)
    {
        this->releaseLock();
        return 1;
    }
    this->releaseLock();
    return 0;
}

RepresentationStreamType	MPDWrapper::determineRepresentationStreamType	(IRepresentation *representation, IAdaptationSet *adaptationSet, IPeriod* period)
{
    /* check on Representation Level */
    if (representation->GetSegmentList())
        return libdash::framework::mpd::SegmentList;

    if (representation->GetSegmentTemplate())
        return libdash::framework::mpd::SegmentTemplate;

    if (representation->GetSegmentBase() || representation->GetBaseURLs().size() > 0)
        return libdash::framework::mpd::SingleMediaSegment;

    /* check on AdaptationSet Level */
    if (adaptationSet->GetSegmentList())
        return libdash::framework::mpd::SegmentList;

    if (adaptationSet->GetSegmentTemplate())
        return libdash::framework::mpd::SegmentTemplate;

    if (adaptationSet->GetSegmentBase())
        return libdash::framework::mpd::SingleMediaSegment;

    /* check on Period Level */
    if (period->GetSegmentList())
        return libdash::framework::mpd::SegmentList;

    if (period->GetSegmentTemplate())
        return libdash::framework::mpd::SegmentTemplate;

    if (period->GetSegmentBase())
        return libdash::framework::mpd::SingleMediaSegment;

    return libdash::framework::mpd::UNDEFINED;
}

void	MPDWrapper::initializeAdaptationSetStream	(viper::managers::StreamType type)
{
    IAdaptationSet *adaptationSet = NULL;
    std::map<dash::mpd::IRepresentation *, IRepresentationStream *> *representations = NULL;
    this->acquireLock();

    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        if(this->audioRepresentations == NULL)
            this->audioRepresentations = new std::map<dash::mpd::IRepresentation *, IRepresentationStream *>();
        if(this->audioAdaptationSet == NULL)
            return;
        adaptationSet = this->audioAdaptationSet;
        representations = this->audioRepresentations;
        break;
    case viper::managers::StreamType::VIDEO:
        if(this->videoRepresentations == NULL)
            this->videoRepresentations = new std::map<dash::mpd::IRepresentation *, IRepresentationStream *>();
        if(this->videoAdaptationSet == NULL)
            return;
        adaptationSet = this->videoAdaptationSet;
        representations = this->videoRepresentations;
        break;
    default:
        return;
    }

    for (size_t i = 0; i < adaptationSet->GetRepresentation().size(); i++)
    {
        IRepresentation *representation = adaptationSet->GetRepresentation().at(i);
        RepresentationStreamType typeR   = determineRepresentationStreamType(representation, adaptationSet, this->period);
        (*representations)[representation] = RepresentationStreamFactory::create(type, typeR, this, period, adaptationSet, representation);
    }
    this->releaseLock();
}

void	MPDWrapper::initializeAdaptationSetStreamWithoutLock	(viper::managers::StreamType type)
{
    IAdaptationSet *adaptationSet = NULL;
    std::map<dash::mpd::IRepresentation *, IRepresentationStream *> *representations = NULL;

    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        if(this->audioRepresentations == NULL)
            this->audioRepresentations = new std::map<dash::mpd::IRepresentation *, IRepresentationStream *>();
        if(this->audioAdaptationSet == NULL)
            return;
        adaptationSet = this->audioAdaptationSet;
        representations = this->audioRepresentations;
        break;
    case viper::managers::StreamType::VIDEO:
        if(this->videoRepresentations == NULL)
            this->videoRepresentations = new std::map<dash::mpd::IRepresentation *, IRepresentationStream *>();
        if(this->videoAdaptationSet == NULL)
            return;
        adaptationSet = this->videoAdaptationSet;
        representations = this->videoRepresentations;
        break;
    default:
        return;
    }
    for (size_t i = 0; i < adaptationSet->GetRepresentation().size(); i++)
    {
        IRepresentation *representation = adaptationSet->GetRepresentation().at(i);
        RepresentationStreamType typeR   = determineRepresentationStreamType(representation, adaptationSet, this->period);
        (*representations)[representation] = RepresentationStreamFactory::create(type, typeR, this, period, adaptationSet, representation);
    }
}

void	MPDWrapper::initializeAdaptationSetStreamWithoutLock	(viper::managers::StreamType type, IMPD* mpd)
{
    IAdaptationSet *adaptationSet = NULL;
    std::map<dash::mpd::IRepresentation *, IRepresentationStream *> *representations = NULL;

    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        if(this->audioRepresentations == NULL)
            this->audioRepresentations = new std::map<dash::mpd::IRepresentation *, IRepresentationStream *>();
        if(this->audioAdaptationSet == NULL)
            return;
        adaptationSet = this->audioAdaptationSet;
        representations = this->audioRepresentations;
        break;
    case viper::managers::StreamType::VIDEO:
        if(this->videoRepresentations == NULL)
            this->videoRepresentations = new std::map<dash::mpd::IRepresentation *, IRepresentationStream *>();
        if(this->videoAdaptationSet == NULL)
            return;
        adaptationSet = this->videoAdaptationSet;
        representations = this->videoRepresentations;
        break;
    default:
        return;
    }
    for (size_t i = 0; i < adaptationSet->GetRepresentation().size(); i++)
    {
        IRepresentation *representation = adaptationSet->GetRepresentation().at(i);
        RepresentationStreamType typeR   = determineRepresentationStreamType(representation, adaptationSet, this->period);
        (*representations)[representation] = RepresentationStreamFactory::create(type, typeR, this, period, adaptationSet, representation, mpd);
    }
}

void	MPDWrapper::destroyAdaptationSetStream	(viper::managers::StreamType type)
{
    std::map<dash::mpd::IRepresentation *, IRepresentationStream *> *representations = NULL;

    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        representations = this->audioRepresentations;
        break;
    case viper::managers::StreamType::VIDEO:
        representations = this->videoRepresentations;
        break;
    default:
        return;
    }

    std::map<IRepresentation *, IRepresentationStream *>::iterator iter;
    for (iter = representations->begin(); iter != representations->end(); ++iter)
    {
        delete(iter->second);
    }
    representations->clear();
}

std::vector<dash::mpd::IBaseUrl *>	MPDWrapper::resolveBaseUrl	(viper::managers::StreamType type, size_t mpdBaseUrl, size_t periodBaseUrl, size_t adaptationSetBaseUrl)
{

    IAdaptationSet *adaptationSet = NULL;
    std::vector<dash::mpd::IBaseUrl *> urls;

    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        adaptationSet = this->audioAdaptationSet;
        break;
    case viper::managers::StreamType::VIDEO:
        adaptationSet = this->videoAdaptationSet;
        break;
    default:
        return urls;
    }

    if(adaptationSet == NULL)
        return urls;

    if (mpd->GetBaseUrls().size() > 0)
    {
        if (mpd->GetBaseUrls().size() > mpdBaseUrl)
            urls.push_back(mpd->GetBaseUrls().at(mpdBaseUrl));
        else
            urls.push_back(mpd->GetBaseUrls().at(0));
    }
    if (period->GetBaseURLs().size() > 0)
    {
        if (period->GetBaseURLs().size() > periodBaseUrl)
            urls.push_back(period->GetBaseURLs().at(periodBaseUrl));
        else
            urls.push_back(period->GetBaseURLs().at(0));
    }
    if (adaptationSet->GetBaseURLs().size() > 0)
    {
        if (adaptationSet->GetBaseURLs().size() > adaptationSetBaseUrl)
            urls.push_back(adaptationSet->GetBaseURLs().at(adaptationSetBaseUrl));
        else
            urls.push_back(adaptationSet->GetBaseURLs().at(0));
    }

    if (urls.size() > 0)
    {
        if (urls.at(0)->GetUrl().substr(0,7) != "http://" && urls.at(0)->GetUrl().substr(0,8) != "https://")
        {
            urls.push_back(mpd->GetMPDPathBaseUrl());
            size_t lastPos = urls.size() - 1;
            IBaseUrl *absoluteUrl = urls.at(lastPos);
            for (size_t i = lastPos; i > 0; i--)
            {
                urls[i] = urls[i-1];
            }
            urls[0] = absoluteUrl;
        }
    }
    else
    {
        urls.push_back(mpd->GetMPDPathBaseUrl());
    }

    return urls;
}

std::vector<dash::mpd::IBaseUrl *>	MPDWrapper::resolveBaseUrl	(viper::managers::StreamType type, size_t mpdBaseUrl, size_t periodBaseUrl, size_t adaptationSetBaseUrl, IMPD* mpd)
{

    IAdaptationSet *adaptationSet = NULL;
    std::vector<dash::mpd::IBaseUrl *> urls;

    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        adaptationSet = this->audioAdaptationSet;
        break;
    case viper::managers::StreamType::VIDEO:
        adaptationSet = this->videoAdaptationSet;
        break;
    default:
        return urls;
    }

    if(adaptationSet == NULL)
        return urls;

    if (mpd->GetBaseUrls().size() > 0)
    {
        if (mpd->GetBaseUrls().size() > mpdBaseUrl)
            urls.push_back(mpd->GetBaseUrls().at(mpdBaseUrl));
        else
            urls.push_back(mpd->GetBaseUrls().at(0));
    }
    if (period->GetBaseURLs().size() > 0)
    {
        if (period->GetBaseURLs().size() > periodBaseUrl)
            urls.push_back(period->GetBaseURLs().at(periodBaseUrl));
        else
            urls.push_back(period->GetBaseURLs().at(0));
    }
    if (adaptationSet->GetBaseURLs().size() > 0)
    {
        if (adaptationSet->GetBaseURLs().size() > adaptationSetBaseUrl)
            urls.push_back(adaptationSet->GetBaseURLs().at(adaptationSetBaseUrl));
        else
            urls.push_back(adaptationSet->GetBaseURLs().at(0));
    }

    if (urls.size() > 0)
    {
        if (urls.at(0)->GetUrl().substr(0,7) != "http://" && urls.at(0)->GetUrl().substr(0,8) != "https://")
        {
            urls.push_back(mpd->GetMPDPathBaseUrl());
            size_t lastPos = urls.size() - 1;
            IBaseUrl *absoluteUrl = urls.at(lastPos);
            for (size_t i = lastPos; i > 0; i--)
            {
                urls[i] = urls[i-1];
            }
            urls[0] = absoluteUrl;
        }
    }
    else
    {
        urls.push_back(mpd->GetMPDPathBaseUrl());
    }

    return urls;
}

void	MPDWrapper::acquireLock	()
{
    EnterCriticalSection(&this->monitorMutex);
}


void	MPDWrapper::releaseLock	()
{
    LeaveCriticalSection(&this->monitorMutex);
}

void	MPDWrapper::setSegmentOffset(viper::managers::StreamType type, uint32_t segmentOffset)
{
    this->acquireLock();
    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        this->audioSegmentOffset = segmentOffset;
        this->audioRepresentations->find(this->audioRepresentation)->second->setSegmentOffset(segmentOffset);
        break;
    case viper::managers::StreamType::VIDEO:
        this->videoSegmentOffset = segmentOffset;
        this->videoRepresentations->find(this->videoRepresentation)->second->setSegmentOffset(segmentOffset);
        break;
    default:
        break;
    }
    this->releaseLock();
}

MediaObject*	MPDWrapper::getNextSegment	(viper::managers::StreamType type, bool isLooping, uint32_t &segmentNumber, bool withFeedBack)
{
    IRepresentation* representation;
    std::map<dash::mpd::IRepresentation *, IRepresentationStream *> *representations;
    
    this->acquireLock();
    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        representation = this->audioRepresentation;
        representations = this->audioRepresentations;
        break;
    case viper::managers::StreamType::VIDEO:
        representation = this->videoRepresentation;
        representations = this->videoRepresentations;
        break;
    default:
        this->releaseLock();
    return NULL;
    }

    ISegment* seg = NULL;
    IRepresentationStream* representationStream = representations->find(representation)->second;

    if(!strcmp(this->mpd->GetType().c_str(), "static"))
    {
        if(segmentNumber >= representationStream->getSize())
        {
            if(isLooping)
            {
                segmentNumber = 0;
            }
            else
            {
                switch(type)
                {
                case viper::managers::StreamType::AUDIO:
                    this->audioSegmentNumber = segmentNumber;
                    break;
                case viper::managers::StreamType::VIDEO:
                    this->videoSegmentNumber = segmentNumber;
                    break;
                default:
                    break;
                }
                this->releaseLock();
                return NULL;
            }
        }
    }
    else
    {
        while((this->isStopping == false) && segmentNumber >= representationStream->getSize())
        {
            SleepConditionVariableCS(&this->mpdUpdate, &this->monitorMutex, INFINITE);

            if(this->isStopping)
            {
                this->releaseLock();
                return NULL;
            }
            //Need to update representationStream here as it was updated with the mpd (Live only):
            switch(type)
            {
                case viper::managers::StreamType::AUDIO:
                    representation = this->audioRepresentation;
                    representations = this->audioRepresentations;
                    break;
                case viper::managers::StreamType::VIDEO:
                    representation = this->videoRepresentation;
                    representations = this->videoRepresentations;
                    break;
                default:
                    break;
            }
            representationStream = representations->find(representation)->second;
        }
    }
    seg = representationStream->getMediaSegment(segmentNumber);
    if(seg != NULL)
    {
        MediaObject *media = new MediaObject(seg, representation, withFeedBack); 
        segmentNumber++;
        switch(type)
        {
        case viper::managers::StreamType::AUDIO:
            this->audioSegmentNumber = segmentNumber;
            this->audioSegmentIsSet = false;
            this->audioSegmentQuality = -1;
            break;
        case viper::managers::StreamType::VIDEO:
            this->videoSegmentNumber = segmentNumber;
            this->videoSegmentIsSet = false;
            this->videoSegmentQuality = -1;
            break;
        default:
            break;
        }
        this->releaseLock();
        return media;
    }
    this->releaseLock();
    return NULL;
}

MediaObject*	MPDWrapper::getSegment	(viper::managers::StreamType type, uint32_t segNum)
{
    IRepresentation* representation;
    std::map<dash::mpd::IRepresentation *, IRepresentationStream *> *representations;
    this->acquireLock();

    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        representation = this->audioRepresentation;
        representations = this->audioRepresentations;
        break;
    case viper::managers::StreamType::VIDEO:
        representation = this->videoRepresentation;
        representations = this->videoRepresentations;
        break;
    default:
        this->releaseLock();
        return NULL;
    }

    ISegment* seg = NULL;
    IRepresentationStream* representationStream = representations->find(representation)->second;
    if(segNum >= representationStream->getSize())
    {
        this->releaseLock();
        return NULL;
    }
    seg = representationStream->getMediaSegment(segNum);
    if(seg != NULL)
    {
        MediaObject *media = new MediaObject(seg, representation);
        this->releaseLock();
        return media;
    }
    this->releaseLock();
    return NULL;
}

MediaObject*	MPDWrapper::getInitSegment	(viper::managers::StreamType type)
{
    IRepresentation* representation;
    std::map<dash::mpd::IRepresentation *, IRepresentationStream *> *representations;
    this->acquireLock();

    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        representation = this->audioRepresentation;
        representations = this->audioRepresentations;
        break;
    case viper::managers::StreamType::VIDEO:
        representation = this->videoRepresentation;
        representations = this->videoRepresentations;
        break;
    default:
        this->releaseLock();
        return NULL;
    }

    ISegment* seg = NULL;
    IRepresentationStream* representationStream = representations->find(representation)->second;
    seg = representationStream->getInitializationSegment();

    if(seg != NULL)
    {
        MediaObject *media = new MediaObject(seg, representation);
        this->releaseLock();
        return media;
    }
    this->releaseLock();
    return NULL;
}

void	MPDWrapper::setQuality(viper::managers::StreamType type, IPeriod* period, IAdaptationSet *adaptationSet, IRepresentation *representation)
{
    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        this->setAudioQuality(period, adaptationSet, representation);
        break;
    case viper::managers::StreamType::VIDEO:
        this->setVideoQuality(period, adaptationSet, representation);
        break;
    default:
        return;
    }
}

void	MPDWrapper::setAudioQuality	(IPeriod *period, IAdaptationSet *adaptationSet, IRepresentation *representation)
{
    bool periodChanged = false;
    if (this->audioRepresentation == representation)
    {
        this->releaseLock();
        return;
    }

    this->audioRepresentation = representation;

    if (this->audioAdaptationSet != adaptationSet)
    {
        this->audioAdaptationSet = adaptationSet;

        if (this->period != period)
        {
            this->period = period;
            periodChanged = true;
        }

        this->destroyAdaptationSetStream(viper::managers::StreamType::AUDIO);
        this->initializeAdaptationSetStreamWithoutLock(viper::managers::StreamType::AUDIO);
    }
}

void	MPDWrapper::setVideoQuality	(IPeriod *period, IAdaptationSet *adaptationSet, IRepresentation *representation)
{
    bool periodChanged = false;
    if (this->videoRepresentation == representation)
    {
        this->releaseLock();
        return;
    }

    this->videoRepresentation = representation;

    if (this->videoAdaptationSet != adaptationSet)
    {
        this->videoAdaptationSet = adaptationSet;

        if (this->period != period)
        {
            this->period = period;
            periodChanged = true;
        }

        this->destroyAdaptationSetStream(viper::managers::StreamType::VIDEO);
        this->initializeAdaptationSetStreamWithoutLock(viper::managers::StreamType::VIDEO);
    }
}

uint32_t	MPDWrapper::calculateSegmentOffset	(viper::managers::StreamType type, uint32_t bufferSize)
{
    IRepresentation* representation;
    std::map<dash::mpd::IRepresentation *, IRepresentationStream *> *representations;
    this->acquireLock();

    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        representation = this->audioRepresentation;
        representations = this->audioRepresentations;
        break;
    case viper::managers::StreamType::VIDEO:
        representation = this->videoRepresentation;
        representations = this->videoRepresentations;
        break;
    default:
        this->releaseLock();
        return 0;
    }

    if(!(strcmp(this->mpd->GetType().c_str(), "static")))
    {
        this->releaseLock();
        return 0;
    }
    IRepresentationStream* representationStream = representations->find(representation)->second;
    uint32_t firstSegNum = representationStream->getFirstSegmentNumber();
    uint32_t currSegNum = representationStream->getCurrentSegmentNumber();
    uint32_t startSegNum = currSegNum - 2*bufferSize;

    this->releaseLock();
    return (startSegNum > firstSegNum) ? startSegNum : firstSegNum;
}

std::string	MPDWrapper::getRepresentationID	(viper::managers::StreamType type)
{
    std::string id = "";
    this->acquireLock();

    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        id = this->audioRepresentation->GetId();
        break;
    case viper::managers::StreamType::VIDEO:
        id = this->videoRepresentation->GetId();
        break;
    default:
        break;
    }
    this->releaseLock();
    return id;
}

std::string	MPDWrapper::getPublishTime	()
{
    this->acquireLock();
    std::string pubTime = this->mpd->GetPublishTime();
    this->releaseLock();
    return pubTime;
}

std::string	MPDWrapper::getMinimumUpdatePeriod	()
{
    this->acquireLock();
    std::string res = this->mpd->GetMinimumUpdatePeriod();
    this->releaseLock();
    return res;
}


/*******************************************
********************************************
*****              CAREFUL             *****
***** These functions should be called *****
***** only if the lock was acquired!!! *****
*****                                  *****
********************************************
*******************************************/

std::vector<IRepresentation *> MPDWrapper::getRepresentations	(viper::managers::StreamType type)
{
    std::vector<IRepresentation *> rep;
    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        if(this->audioAdaptationSet)
            return this->audioAdaptationSet->GetRepresentation();
        return rep;
    case viper::managers::StreamType::VIDEO:
        if(this->videoAdaptationSet)
            return this->videoAdaptationSet->GetRepresentation();
        return rep;
    default:
        return rep;
    }
}

std::string	MPDWrapper::getMediaPresentationDuration	()
{
    return this->mpd->GetMediaPresentationDuration();
}

IRepresentation*	MPDWrapper::getRepresentationAt	(viper::managers::StreamType type, int index)
{
    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        if(this->audioAdaptationSet)
            return this->audioAdaptationSet->GetRepresentation().at(index);
        return NULL;
    case viper::managers::StreamType::VIDEO:
        if(this->videoAdaptationSet)
            return this->videoAdaptationSet->GetRepresentation().at(index);
        return NULL;
    default:
        return NULL;
    }
}

void	MPDWrapper::setRepresentation	(viper::managers::StreamType type, IRepresentation* rep)
{
    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        this->audioRepresentation = rep;
        return;
    case viper::managers::StreamType::VIDEO:
        this->videoRepresentation = rep;
        return;
    default:
        break;
    }
}

std::string	MPDWrapper::getRepresentationIDWithoutLock	(viper::managers::StreamType type)
{
    std::string id = "";
    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        id = this->audioRepresentation->GetId();
        break;
    case viper::managers::StreamType::VIDEO:
        id = this->videoRepresentation->GetId();
        break;
    default:
        break;
    }
    return id;
}

MediaObject*	MPDWrapper::getInitSegmentWithoutLock	(viper::managers::StreamType type)
{
    IRepresentation* representation;
    std::map<dash::mpd::IRepresentation *, IRepresentationStream *> *representations;

    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        representation = this->audioRepresentation;
        representations = this->audioRepresentations;
        break;
    case viper::managers::StreamType::VIDEO:
        representation = this->videoRepresentation;
        representations = this->videoRepresentations;
        break;
    default:
        return NULL;
    }
    ISegment* seg = NULL;
    IRepresentationStream* representationStream = representations->find(representation)->second;
    seg = representationStream->getInitializationSegment();

    if(seg != NULL)
    {
        MediaObject *media = new MediaObject(seg, representation);
        return media;
    }
    return NULL;
}

std::string	MPDWrapper::getAvailabilityStarttime	()
{
    return this->mpd->GetAvailabilityStarttime();
}

std::string	MPDWrapper::getTimeShiftBufferDepth	()
{
    return this->mpd->GetTimeShiftBufferDepth();
}

std::string	MPDWrapper::getTypeWithoutLock	()
{
    return this->mpd->GetType();
}

std::string	MPDWrapper::getMinimumUpdatePeriodWithoutLock	()
{
    return this->mpd->GetMinimumUpdatePeriod();
}

uint32_t	MPDWrapper::getFetchTime	()
{
     return this->mpd->GetFetchTime();
}

void	MPDWrapper::settingsChanged	(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation)
{
    this->acquireLock();
    this->period = this->mpd->GetPeriods().at(period);
    std::vector<IAdaptationSet *>   videoAdaptationSets = AdaptationSetHelper::getVideoAdaptationSets(this->period);
    std::vector<IAdaptationSet *>   audioAdaptationSets = AdaptationSetHelper::getAudioAdaptationSets(this->period);
    if (videoAdaptationSet >= 0 && videoRepresentation >= 0 && !videoAdaptationSets.empty())
    {
        this->videoAdaptationSet = videoAdaptationSets.at(videoAdaptationSet);
        this->videoRepresentation = this->videoAdaptationSet->GetRepresentation().at(videoRepresentation);
    }
    else
    {
        this->videoAdaptationSet = NULL;
        this->videoRepresentation = NULL;
    }
    if (audioAdaptationSet >= 0 && audioRepresentation >= 0 && !audioAdaptationSets.empty())
    {
        this->audioAdaptationSet = audioAdaptationSets.at(audioAdaptationSet);
        this->audioRepresentation = this->audioAdaptationSet->GetRepresentation().at(audioRepresentation);
    }
    else
    {
        this->audioAdaptationSet = NULL;
        this->audioRepresentation = NULL;
    }
    this->releaseLock();
}

//Returns the segmentDuration
float	MPDWrapper::onFirstDownloadMPD	(viper::IViperGui *gui)
{
    float segmentDuration = 0.0;
    this->acquireLock();
    IRepresentation *representation = this->videoAdaptationSet->GetRepresentation().at(0);
    if(!strcmp(this->mpd->GetType().c_str(), "static")) // VOD MPD
    {
        if(representation->GetSegmentList())
        {
            uint32_t duration = representation->GetSegmentList()->GetDuration();
            uint32_t timescale = representation->GetSegmentList()->GetTimescale();
            segmentDuration = 1.0*duration/(1.0*timescale) * 1000;
            if(gui)
            {
                gui->setListSegmentSize(representation->GetSegmentList()->GetSegmentURLs().size());
                gui->setSegmentDuration(segmentDuration);
            }
        }
        else //SegmentTemplate
        {
            uint32_t duration = 0;
            uint32_t timescale = 0;

            //check if segmentTemplate is on the representation
            if(representation->GetSegmentTemplate())
            {
                duration = representation->GetSegmentTemplate()->GetDuration();
                timescale = representation->GetSegmentTemplate()->GetTimescale();
            }
            else //check at adaptationSet then
            {
                if(this->videoAdaptationSet->GetSegmentTemplate())
                {
                    duration = this->videoAdaptationSet->GetSegmentTemplate()->GetDuration();
                    timescale = this->videoAdaptationSet->GetSegmentTemplate()->GetTimescale();
                }
                else //check at period
                {
                    if(this->period->GetSegmentTemplate())
                    {
                        duration = this->period->GetSegmentTemplate()->GetDuration();
                        timescale = this->period->GetSegmentTemplate()->GetTimescale();
                    }
                }
            }
            segmentDuration = 1.0*duration/(1.0*timescale) * 1000;

            if(gui)
            {
                gui->setSegmentDuration(segmentDuration);
                gui->setListSegmentSize(TimeResolver::getDurationInSec(period->GetDuration())*1000/segmentDuration + 1);
            }
        }
    }
    else   //Live MPD
    {
        //Assuming here that the segment duration doesn't change. If so, need to do an average over all segments.
        uint32_t duration = 0;
        uint32_t timescale = 0;
        if(representation->GetSegmentTemplate())
        {
            duration = representation->GetSegmentTemplate()->GetDuration();
            timescale = representation->GetSegmentTemplate()->GetTimescale();
        }
        else
        {
            if(this->videoAdaptationSet->GetSegmentTemplate())  //SegmentTemplate at AdaptationSet level
            {
                duration = this->videoAdaptationSet->GetSegmentTemplate()->GetDuration();
                timescale = this->videoAdaptationSet->GetSegmentTemplate()->GetTimescale();
            }
            else
            { // SegmentTemplate at Period level
                duration = this->period->GetSegmentTemplate()->GetDuration();
                timescale = this->period->GetSegmentTemplate()->GetTimescale();
            }
        }
        segmentDuration = 1.0*duration/(1.0*timescale) * 1000;
        if(gui)
        {
            gui->setSegmentDuration(segmentDuration);
            gui->setListSegmentSize(0);
        }
    }
    this->releaseLock();
    return segmentDuration;
}

void    MPDWrapper::setIsStopping   (bool isStopping)
{
    this->isStopping = isStopping;
    WakeAllConditionVariable(&this->mpdUpdate);
}

void MPDWrapper::setSegmentIsSetFlag(viper::managers::StreamType type, bool flag)
{
    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        this->audioSegmentIsSet = flag;
        break;
    case viper::managers::StreamType::VIDEO:
        this->videoSegmentIsSet = flag;
        break;
    default:
        break;
    }
}

bool MPDWrapper::getSegmentIsSetFlag(viper::managers::StreamType type)
{
    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        return this->audioSegmentIsSet;
    case viper::managers::StreamType::VIDEO:
        return this->videoSegmentIsSet;
    default:
        return false;
    }
}

int MPDWrapper::getSegmentQuality(viper::managers::StreamType type)
{
    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        return this->audioSegmentQuality;
    case viper::managers::StreamType::VIDEO:
        return this->videoSegmentQuality;
    default:
        return false;
    }
}

void MPDWrapper::setSegmentQuality(viper::managers::StreamType type, int segQuality)
{
    switch(type)
    {
    case viper::managers::StreamType::AUDIO:
        this->audioSegmentIsSet = segQuality;
        break;
    case viper::managers::StreamType::VIDEO:
        this->videoSegmentIsSet = segQuality;
        break;
    default:
        break;
    }
}
