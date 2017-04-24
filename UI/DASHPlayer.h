/*
 * DASHPlayer.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef DASHPLAYER_H_
#define DASHPLAYER_H_

#include <iostream>
#include <sstream>
#include <qobject.h>
#include "libdash.h"
#include "IDASHPlayerGuiObserver.h"
#include "../Managers/IMultimediaManagerObserver.h"
#include "../Managers/MultimediaManager.h"
#include "../Adaptation/IAdaptationLogic.h"
#include "../Buffer/IBufferObserver.h"
#include "../MPD/AdaptationSetHelper.h"
#include "../Common/Config.h"
#include <qimage.h>
#include<map>
#include<tuple>

namespace viper
{
struct settings_t
{
    int period;
    int videoAdaptationSet;
    int audioAdaptationSet;
    int videoRepresentation;
    int audioRepresentation;
};

class DASHPlayer : public IDASHPlayerGuiObserver, public managers::IMultimediaManagerObserver

{
    Q_OBJECT

public:
    DASHPlayer(ViperGui& gui, Config *config);
    virtual ~DASHPlayer();

    virtual void onSettingsChanged(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation);
    virtual void onStartButtonPressed(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation, int adaptationLogic);
    virtual void stopButtonPressed();
    virtual void onPauseButtonPressed();
    virtual void onVideoBufferStateChanged(uint32_t fillstateInPercent);
    virtual void onVideoSegmentBufferStateChanged(uint32_t fillstateInPercent);
    virtual void onAudioBufferStateChanged(uint32_t fillstateInPercent);
    virtual void onAudioSegmentBufferStateChanged(uint32_t fillstateInPercent);
    virtual void onEOS();
    virtual void notifyStatistics(int, uint32_t, int, uint32_t);
    virtual void notifyQualityDownloading(uint32_t);
    virtual bool onDownloadMPDPressed(const std::string &url);
    void setConfig(Config *config);
    Q_INVOKABLE bool downloadMPD(const QString &url, const QString &adaptationLogic, bool icn);
    Q_INVOKABLE void pause();
    Q_INVOKABLE void seekVideo(float value);
    Q_INVOKABLE void repeatVideo(bool repeat);
    Q_INVOKABLE void onStopButtonPressed();
    Q_INVOKABLE void play();
    Q_INVOKABLE void onStopped();
    Q_INVOKABLE QString getLastPlayed();
    Q_INVOKABLE void setLastPlayed(QString lastPlayed);
    Q_INVOKABLE QString getAdaptationLogic();
    Q_INVOKABLE void setAdaptationLogic(QString adaptationLogic);
    Q_INVOKABLE bool getIcn();
    Q_INVOKABLE void setIcn(bool icn);
    Q_INVOKABLE QString getIcnPrefix();
    Q_INVOKABLE void setIcnPrefix(QString icnPrefix);
    Q_INVOKABLE QString getHttpPrefix();
    Q_INVOKABLE void setHttpPrefix(QString httpPrefix);
    Q_INVOKABLE QString getIcnSuffix();
    Q_INVOKABLE void setIcnSuffix(QString icnSuffix);
    Q_INVOKABLE QString getHttpSuffix();
    Q_INVOKABLE void setHttpSuffix(QString httpSuffix);
    Q_INVOKABLE qreal getAlpha();
    Q_INVOKABLE void setAlpha(qreal alpha);
    Q_INVOKABLE qreal getSegmentBufferSize();
    Q_INVOKABLE void setSegmentBufferSize(qreal segmentBufferSize);
    Q_INVOKABLE qreal getRateAlpha();
    Q_INVOKABLE void setRateAlpha(qreal rateAlpha);
    Q_INVOKABLE qreal getBufferReservoirThreshold();
    Q_INVOKABLE void setBufferReservoirThreshold(qreal bufferReservoirThreshold);
    Q_INVOKABLE qreal getBufferMaxThreshold();
    Q_INVOKABLE void setBufferMaxThreshold(qreal bufferMaxThreshold);
    Q_INVOKABLE qreal getAdaptechFirstThreshold();
    Q_INVOKABLE void setAdaptechFirstThreshold(qreal adaptechFirstThreshold);
    Q_INVOKABLE qreal getAdaptechSecondThreshold();
    Q_INVOKABLE void setAdaptechSecondThreshold(qreal adaptechSecondThreshold);
    Q_INVOKABLE qreal getAdaptechSwitchUpMargin();
    Q_INVOKABLE void setAdaptechSwitchUpMargin(qreal adaptechSwitchUpMargin);
    Q_INVOKABLE qreal getAdaptechSlackParameter();
    Q_INVOKABLE void setAdaptechSlackParameter(qreal adaptechSlackParameter);
    Q_INVOKABLE qreal getAdaptechAlpha();
    Q_INVOKABLE void setAdaptechAlpha(qreal adaptechAlpha);
    Q_INVOKABLE qreal getBufferThreeThresholdFirst();
    Q_INVOKABLE void setBufferThreeThresholdFirst(qreal bufferThreeThresholdFirst);
    Q_INVOKABLE qreal getBufferThreeThresholdSecond();
    Q_INVOKABLE void setBufferThreeThresholdSecond(qreal bufferThreeThresholdSecond);
    Q_INVOKABLE qreal getBufferThreeThresholdThird();
    Q_INVOKABLE void setBufferThreeThresholdThird(qreal bufferThreeThresholdThird);
    Q_INVOKABLE qreal getPandaParamAlpha();
    Q_INVOKABLE void setPandaParamAlpha(qreal pandaParamAlpha);
    Q_INVOKABLE qreal getPandaParamBeta();
    Q_INVOKABLE void setPandaParamBeta(qreal pandaParamBeta);
    Q_INVOKABLE qreal getPandaParamBMin();
    Q_INVOKABLE void setPandaParamBMin(qreal pandaParamBMin);
    Q_INVOKABLE qreal getPandaParamK();
    Q_INVOKABLE void setPandaParamK(qreal pandaParamK);
    Q_INVOKABLE qreal getPandaParamW();
    Q_INVOKABLE void setPandaParamW(qreal pandaParamW);
    Q_INVOKABLE qreal getPandaParamEpsilon();
    Q_INVOKABLE void setPandaParamEpsilon(qreal pandaParamEpsilon);
    Q_INVOKABLE qreal getBolaBufferTarget();
    Q_INVOKABLE void setBolaBufferTarget(qreal bolaBufferTarget);
    Q_INVOKABLE qreal getBolaAlpha();
    Q_INVOKABLE void setBolaAlpha(qreal bolaAlpha);
    Q_INVOKABLE bool getRepeat();
    Q_INVOKABLE void setRepeat(bool repeat);
    Q_INVOKABLE bool getGraph();
    Q_INVOKABLE void setGraph(bool graph);
    Q_INVOKABLE bool getFullScreen();
    Q_INVOKABLE void setFullScreen(bool fullScreen);
    Q_INVOKABLE void reloadParameters();
    Q_INVOKABLE bool getStop();
    Q_INVOKABLE void setAutotune(bool autoTune);
    Q_INVOKABLE bool getAutotune();
    Q_INVOKABLE void setLifetime(int lifeTime);
    Q_INVOKABLE int getLifetime();
    Q_INVOKABLE void setRetransmissions(int retranmsissions);
    Q_INVOKABLE int getRetransmissions();
    Q_INVOKABLE void setBeta(qreal beta);
    Q_INVOKABLE qreal getBeta();
    Q_INVOKABLE void setDrop(qreal drop);
    Q_INVOKABLE qreal getDrop();
    Q_INVOKABLE void setBetaWifi(qreal betaWifi);
    Q_INVOKABLE qreal getBetaWifi();
    Q_INVOKABLE void setDropWifi(qreal dropWifi);
    Q_INVOKABLE qreal getDropWifi();
    Q_INVOKABLE void setDelayWifi(int delayWifi);
    Q_INVOKABLE int getDelayWifi();
    Q_INVOKABLE void setBetaLte(qreal betaLte);
    Q_INVOKABLE qreal getBetaLte();
    Q_INVOKABLE void setDropLte(qreal dropLte);
    Q_INVOKABLE qreal getDropLte();
    Q_INVOKABLE void setDelayLte(int delayLte);
    Q_INVOKABLE int getDelayLte();
    Q_INVOKABLE void setBatchingParameter(int batchingParameter);
    Q_INVOKABLE int getBatchingParameter();
    Q_INVOKABLE void setRateEstimator(int rateEstimator);
    Q_INVOKABLE int getRateEstimator();

private:
    float                                       gamma;
    float                                       beta;
    float                                       drop;
    bool                                        seek;
    Config                                      *config;
    bool                                        repeat;
    float                                       segmentDuration;
    uint64_t                                    offset;
    uint64_t                                    position;
    int                                         segment;
    int                                         adaptationLogic;
    dash::mpd::IMPD                             *mpd;
    ViperGui									*gui = NULL;
    viper::managers::MultimediaManager          *multimediaManager;
    settings_t									currentSettings;
    CRITICAL_SECTION							monitorMutex;
    const char									*url;
    bool										icn;
    std::string                                 icnPrefix;
    std::string                                 httpPrefix;
    std::string                                 icnSuffix;
    std::string                                 httpSuffix;
    double                                      alpha;
    struct libdash::framework::adaptation::AdaptationParameters *parametersAdaptation;
    libdash::framework::adaptation::LogicType 	adaptLogic;
    std::map<int,std::tuple<uint32_t, int, uint32_t>>				mStats;
    int											qualityDownloading;
    bool settingsChanged(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation);
    void setSettings(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation);
    std::string msec2string(uint64_t milliseconds);
    void initSlider();

signals:
    void videoSegmentBufferFillStateChanged(int fillStateInPercent);
    void videoBufferFillStateChanged(int fillStateInPercent);
    void audioSegmentBufferFillStateChanged(int fillStateInPercent);
    void audioBufferFillStateChanged(int fillStateInPercent);

private Q_SLOTS:
    void updateSlider(qint64 value);
    void manageGraph(QtAV::AVPlayer::State state);
    void error(const QtAV::AVError &e);
};
}
#endif /* DASHPLAYER_H_ */
