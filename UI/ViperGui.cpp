/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <QtWidgets>
#include <vector>
#include <sstream>
#include "ViperGui.h"
#include "IDASHPlayerGuiObserver.h"
#include "libdash.h"
#include <iostream>
#include <sstream>
#include <QQmlEngine>
#include <QQmlComponent>

using namespace viper;
using namespace dash::mpd;
using namespace libdash::framework::mpd;

ViperGui::ViperGui(QObject *parent) :
    QObject (parent),
    mpd     (NULL)
{
    this->segment = 0;
    this->bufferDuration = 0;
    this->play = false;
    this->pause = false;
    this->stop = true;
    this->offset = 0;
    this->position = 0;
    this->videoPlayer = qvariant_cast<QtAV::AVPlayer *>(parent->property("mediaObject"));
    this->streamBuffer = new ViperBuffer();
    pthread_mutex_init(&(this->monitorMutex), NULL);
    if (streamBuffer->open(QIODevice::ReadWrite)) {
        this->videoPlayer->setIODevice(streamBuffer);
    }
}

ViperGui::~ViperGui()
{
    this->videoPlayer->stop();
    pthread_mutex_destroy(&(this->monitorMutex));
}

void ViperGui::setGuiFields(dash::mpd::IMPD* mpd)
{
    this->setPeriodComboBox(mpd);
    if (mpd->GetPeriods().size() > 0)
    {
        IPeriod *period = mpd->GetPeriods().at(0);
        this->setVideoAdaptationSetComboBox(period);
        if (!AdaptationSetHelper::getVideoAdaptationSets(period).empty())
        {
            IAdaptationSet *adaptationSet = AdaptationSetHelper::getVideoAdaptationSets(period).at(0);
            this->setRepresentationComoboBox(adaptationSet);
        }
        if (!AdaptationSetHelper::getAudioAdaptationSets(period).empty())
        {
            IAdaptationSet *adaptationSet = AdaptationSetHelper::getAudioAdaptationSets(period).at(0);
            this->setRepresentationComoboBox(adaptationSet);
        }
    }
    parse8601(mpd->GetMediaPresentationDuration());
    this->mpd = mpd;
    this->lifeLabel->setProperty("text", QVariant(this->durationString.c_str()));
}

void ViperGui::parse8601(std::string durationISO8601)
{
    int hours, min, sec, millisec;
    sscanf(durationISO8601.c_str(), "PT%dH%dM%d.%dS", &hours, &min, &sec, &millisec);
    this->durationMilliseconds = millisec + 1000*( + sec + 60 *(min + hours * 60));
    char timeStamp[10];
    sprintf(timeStamp, "%02d:%02d:%02d", hours, min, sec);
    this->durationString.assign(timeStamp);
}

void ViperGui::setRepresentationComoboBox(dash::mpd::IAdaptationSet *adaptationSet)
{
    std::vector<IRepresentation *> represenations = adaptationSet->GetRepresentation();
    for(size_t i = 0; i < represenations.size(); i++)
    {
        IRepresentation *representation = represenations.at(i);
    }
}
void ViperGui::setAdaptationSetComboBox(dash::mpd::IPeriod *period)
{
    std::vector<IAdaptationSet *> adaptationSets = period->GetAdaptationSets();
    for(size_t i = 0; i < adaptationSets.size(); i++)
    {
        IAdaptationSet *adaptationSet = adaptationSets.at(i);
    }
}

void ViperGui::setAudioAdaptationSetComboBox(dash::mpd::IPeriod *period)
{
    std::vector<IAdaptationSet *> adaptationSets = AdaptationSetHelper::getAudioAdaptationSets(period);
    for(size_t i = 0; i < adaptationSets.size(); i++)
    {
        IAdaptationSet *adaptationSet = adaptationSets.at(i);
    }
}

void ViperGui::setVideoAdaptationSetComboBox(dash::mpd::IPeriod *period)
{
    std::vector<IAdaptationSet *> adaptationSets = AdaptationSetHelper::getVideoAdaptationSets(period);
    for(size_t i = 0; i < adaptationSets.size(); i++)
    {
        IAdaptationSet *adaptationSet = adaptationSets.at(i);
    }
}
void ViperGui::setPeriodComboBox(dash::mpd::IMPD *mpd)
{
    std::vector<IPeriod *> periods = mpd->GetPeriods();
    for(size_t i = 0; i < periods.size(); i++)
    {
        IPeriod *period = periods.at(i);
    }
}

ViperBuffer* ViperGui::getStreamBuffer()
{
    return this->streamBuffer;
}


QtAV::AVPlayer* ViperGui::getVideoPlayer()
{
    return this->videoPlayer;
}

void ViperGui::setOffset(int offset)
{
    this->offset = offset;
}

void ViperGui::setPosition(qint64 position)
{
    pthread_mutex_lock(&(this->monitorMutex));
    this->position = position;
    pthread_mutex_unlock(&(this->monitorMutex));

}

void ViperGui::setLifeLabel(QObject *lifeLabel)
{
    this->lifeLabel = lifeLabel;
}


QObject* ViperGui::getLifeLabel()
{
    return this->lifeLabel;
}

void ViperGui::setNowLabel(QObject *nowLabel)
{
    this->nowLabel = nowLabel;
}

QObject* ViperGui::getNowLabel(){
    return this->nowLabel;
}

void ViperGui::setPlayButton(QObject *playButton)
{
    this->playButton = playButton;
}

void ViperGui::setProgressBar(QObject *progressBar)
{
    this->progressBar = progressBar;
}

QObject* ViperGui::getProgressBar()
{
    return this->progressBar;
}

void ViperGui::initSlider()
{
    this->offset = 0;
    this->progressBar->setProperty("value", 0.0);
    this->nowLabel->setProperty("text", QVariant("00:00:00"));
    this->lifeLabel->setProperty("text", this->lifeLabel->property("text"));

}


void ViperGui::seekSegment(int segment)
{
    pthread_mutex_lock(&(this->monitorMutex));
    this->segment = segment;
    this->offset = this->segment * this->segmentDuration;
    this->position = this->offset;
    this->bufferDuration = this->segment * this->segmentDuration;
    pthread_mutex_unlock(&(this->monitorMutex));
}

uint64_t ViperGui::getDurationMilliseconds()
{
    return this->durationMilliseconds;
}

void ViperGui::initVideoPlayer()
{
    this->videoPlayer->stop();
    this->streamBuffer->clear();
}

void ViperGui::setVideoStream(managers::MultimediaStream *videoStream)
{
    this->videoStream = videoStream;
}

void ViperGui::clearGraph()
{
    this->graphDataSource->clearData();
}

void ViperGui::setAnaliticsValues(uint32_t bitRate, int fps, uint32_t quality, double bufferSize)
{
    this->graphDataSource->setAnaliticsValues(bitRate, fps, quality, bufferSize);
}

void ViperGui::setGraphDataSource(GraphDataSource *graphDataSource){
    this->graphDataSource = graphDataSource;
}

void ViperGui::writeData(libdash::framework::input::MediaObject* media)
{
    this->streamBuffer->writeData(media);
    pthread_mutex_lock(&(this->monitorMutex));
    this->segment = (this->segment + 1) % this->listSegmentSize;
    if( this->segment > 0)
    {
        this->bufferDuration += this->segmentDuration;

        if(this->bufferDuration - this->position  > 3000)
        {
            if (this->play == true)
            {
                this->videoPlayer->pause(false);
                this->videoPlayer->play();
                QMetaObject::invokeMethod(this->rootObject, "unSetBuffering");
            }
        }
    }
    else
    {
        this->bufferDuration += (this->durationMilliseconds - (this->segmentDuration * (this->listSegmentSize - 1)));

        if(this->bufferDuration - this->position >3000)
        {
            this->videoPlayer->pause(false);
            this->videoPlayer->play();
            QMetaObject::invokeMethod(this->rootObject, "unSetBuffering");
        }
        else
        {
            if (this->play == true)
            {
                this->videoPlayer->pause(false);
                this->videoPlayer->play();
                QMetaObject::invokeMethod(this->rootObject, "unSetBuffering");
            }
        }
        this->streamBuffer->writeToNextBuffer();
    }
    pthread_mutex_unlock(&(this->monitorMutex));

}

void ViperGui::setListSegmentSize(int listSegmentSize)
{
    this->listSegmentSize = listSegmentSize;
}

void ViperGui::setPlay(bool play)
{
    this->play = play;
}

bool ViperGui::getPlay()
{
    return this->play;
}

void ViperGui::setStop(bool stop)
{
    this->stop = stop;
    this->segment = 0;
    this->bufferDuration = 0;
}

bool ViperGui::getStop()
{
    return this->stop;
}

void ViperGui::setPause(bool pause)
{
    this->pause = pause;
}

bool ViperGui::getPause()
{
    return this->pause;
}

void ViperGui::setRepeat(bool repeat)
{
    this->repeat = repeat;
}

void ViperGui::setSegmentDuration(qint64 segmentDuration)
{
    this->segmentDuration = segmentDuration;
}

int64_t ViperGui::getSegmentDuration()
{
    return this->segmentDuration;
}

int64_t ViperGui::getLastSegmentDuration()
{
    return this->lastSegmentDuration;
}


int64_t ViperGui::getBufferDuration()
{
    int64_t bufferDuration;
    pthread_mutex_lock(&(this->monitorMutex));
    bufferDuration = this->bufferDuration - this->position;
    pthread_mutex_unlock(&(this->monitorMutex));
    return bufferDuration;
}

void ViperGui::pauseIfBuffering(qint64 position)
{
    pthread_mutex_lock(&(this->monitorMutex));
    this->position = position;
    if (this->videoPlayer->isPlaying()) {
        if (this->segment == 0) {
            if (this->repeat == true && this->bufferDuration - this->position <= 3000) {
                this->videoPlayer->pause(true);
                QMetaObject::invokeMethod(this->rootObject, "setBuffering");
            }
        } else
        {
            if ((this->bufferDuration - this->position) <= 3000)
            {
                this->videoPlayer->pause(true);
                QMetaObject::invokeMethod(this->rootObject, "setBuffering");
            }
        }
    }
    pthread_mutex_unlock(&(this->monitorMutex));
}

void ViperGui::startIfRepeat()
{
    if(this->play) {
        pthread_mutex_lock(&(this->monitorMutex));
        this->videoPlayer->setStartPosition(0);
        this->bufferDuration -= this->durationMilliseconds;
        this->position = 0;
        this->offset = 0;
        this->videoPlayer->play();
        pthread_mutex_unlock(&(this->monitorMutex));
    }
}

void ViperGui::setRootObject(QObject *rootObject)
{
    this->rootObject = rootObject;
}

QObject* ViperGui::getRootObject()
{
    return this->rootObject;
}

qint64 ViperGui::getPosition()
{

    qint64 position;
    pthread_mutex_lock(&(this->monitorMutex));
    position = this->position;
    pthread_mutex_unlock(&(this->monitorMutex));
    return position;
}

void ViperGui::resetGraphValues() {
    this->graphDataSource->resetGraphValues();
}





