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

#ifndef VIPER_H
#define VIPER_H

#include <QtMultimedia/qmediaplayer.h>
#include <QtMultimediaWidgets/qvideowidget.h>
#include <QtGui/QMovie>
#include <QtAV/QtAV.h>
#include <QByteArray>
#include "Common/ViperBuffer.h"
#include <QBuffer>
#include "libdash.h"
#include "../MPD/AdaptationSetHelper.h"
#include "../Common/QtQuick2ApplicationViewer.h"
#include "../Managers/MultimediaStream.h"
#include "GraphDataSource.h"

#include <QLabel>

namespace viper
{
class IDASHPlayerGuiObserver;

class ViperGui : public QObject
{
    Q_OBJECT

public:
    ViperGui(QObject *parent = 0);
    virtual ~ViperGui  ();
    void seekSegment(int segment);
    void writeData(libdash::framework::input::MediaObject* media);
    void setListSegmentSize(int listSegmentSize);
    ViperBuffer* getStreamBuffer();
    QtAV::AVPlayer* getVideoPlayer();
    void setGuiFields(dash::mpd::IMPD* mpd);
    void setLifeLabel(QObject *lifeLabel);
    void setNowLabel(QObject *nowLabel);
    void setPlayButton(QObject *playButton);
    void setProgressBar(QObject *progressBar);
    void seekVideo(float value);
    uint64_t getDurationMilliseconds();
    void initVideoPlayer();
    void setOffset(int offset);
    void setPosition(qint64 position);
    qint64 getPosition();
    void initSlider();
    void setVideoStream(managers::MultimediaStream *videoStream);
    QObject* getLifeLabel();

    QObject* getNowLabel();
    QObject* getProgressBar();
    void clearGraph();
    void setAnaliticsValues(uint32_t bitRate, int fps, uint32_t quality, double bufferSize);
    void setGraphDataSource(GraphDataSource *graphDataSource);
    void setPlay(bool play);
    bool getPlay();
    void setStop(bool stop);
    bool getStop();
    void setPause(bool pause);
    bool getPause();
    void setRepeat(bool repeat);
    void setSegmentDuration(qint64 segmentDuration);
    void initPlayer(int segmentOffset);
    void pauseIfBuffering(qint64 position);
    void startIfRepeat();
    int64_t getLastSegmentDuration();
    int64_t getBufferDuration();
    int64_t getSegmentDuration();
    void setRootObject(QObject *rootObject);
    QObject *getRootObject();
    void resetGraphValues();

private:
    pthread_mutex_t                     monitorMutex;
    GraphDataSource                     *graphDataSource;
    managers::MultimediaStream          *videoStream;
    int64_t                             offset;
    int64_t                             durationMilliseconds;
    qint64                              position;
    std::string                         durationString;
    QtAV::AVPlayer                      *videoPlayer;
    ViperBuffer                         *streamBuffer;
    std::map<std::string, std::string>  keyValues;
    std::map<std::string, int>          keyIndices;
    std::map<std::string, std::vector<std::string>>    video;
    std::map<std::string, std::vector<std::string>>    audio;
    QObject                             *lifeLabel;
    QObject                             *nowLabel;
    QObject                             *progressBar;
    QObject                             *playButton;
    std::vector<IDASHPlayerGuiObserver *>   observers;
    dash::mpd::IMPD                         *mpd;
    void setPeriodComboBox(dash::mpd::IMPD *mpd);
    void setAdaptationSetComboBox(dash::mpd::IPeriod *period);
    void setVideoAdaptationSetComboBox(dash::mpd::IPeriod *period);
    void setAudioAdaptationSetComboBox(dash::mpd::IPeriod *period);
    void setRepresentationComoboBox(dash::mpd::IAdaptationSet *adaptationSet);
    void parse8601(std::string durationISO8601);
    int                                 listSegmentSize;
    int                                 segment;
    int64_t                             bufferDuration;
    int64_t                             segmentDuration;
    int64_t                             lastSegmentDuration;
    bool                                play;
    bool                                stop;
    bool                                pause;
    bool                                repeat;
    QObject                             *rootObject;

};
}

#endif // VIPER_H
