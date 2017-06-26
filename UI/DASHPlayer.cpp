/*
 * DASHPlayer.cpp
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#include "DASHPlayer.h"
#include <iostream>

using namespace libdash::framework::adaptation;
using namespace libdash::framework::mpd;
using namespace libdash::framework::buffer;
using namespace viper;
using namespace viper::managers;
using namespace dash::mpd;
using namespace std;

DASHPlayer::DASHPlayer(ViperGui &gui, Config *config) :
    gui         (&gui),
    config      (config)
{
    InitializeCriticalSection(&this->monitorMutex);
    this->offset = 0;
    this->url = NULL;
    this->icn = false;
    this->adaptLogic = LogicType::RateBased;
    this->seek = false;
    this->reloadParameters();
    this->setSettings(0, 0, 0, 0, 0);
    this->multimediaManager = new MultimediaManager(this->gui, this->parametersAdaptation->segmentBufferSize, config->getConfigPath().toStdString() + QString::fromLatin1("/").toStdString());
    this->multimediaManager->setBeta(config->beta());
    this->multimediaManager->setDrop(config->drop());
    connect(this->gui->getVideoPlayer(), SIGNAL(positionChanged(qint64)), SLOT(updateSlider(qint64)));
    connect(this->gui->getVideoPlayer(), SIGNAL(stateChanged(QtAV::AVPlayer::State)), SLOT(manageGraph(QtAV::AVPlayer::State)));
    connect(this->gui->getVideoPlayer(), SIGNAL(error(QtAV::AVError)), this, SLOT(error(QtAV::AVError)));
    this->multimediaManager->attachManagerObserver(this);
    this->mpdWrapper = new MPDWrapper(NULL);
    this->multimediaManager->setMPDWrapper(this->mpdWrapper);
}

DASHPlayer::~DASHPlayer()
{
    this->multimediaManager->stop();
    delete(this->multimediaManager);
    if(this->mpdWrapper)
        delete(this->mpdWrapper);
    DeleteCriticalSection(&this->monitorMutex);
}

void DASHPlayer::setMPDWrapper(MPDWrapper* mpdWrapper)
{
    this->mpdWrapper = mpdWrapper;
}

void DASHPlayer::onStartButtonPressed(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation, int adaptationLogic)
{
    bool setOk = false;
    setOk = this->multimediaManager->setVideoAdaptationLogic((LogicType)adaptationLogic, parametersAdaptation);

    if(!setOk)
    {
        return;
    }
    Debug("DASH PLAYER:	STARTING VIDEO\n");
    this->multimediaManager->start(this->icn, 20, 0);
}
void DASHPlayer::stopButtonPressed()
{
    this->multimediaManager->stop();
}

void DASHPlayer::onStopButtonPressed()
{
    QMetaObject::invokeMethod(this->gui->getRootObject(), "unSetBuffering");
    this->gui->setPlay(false);
    this->gui->setStop(true);
    this->gui->setPause(false);
    this->gui->getVideoPlayer()->stop();
    this->stopButtonPressed();
    this->multimediaManager->setOffset(0);
    this->gui->resetGraphValues();
}

void DASHPlayer::onPauseButtonPressed()
{
    this->multimediaManager->onPausePressed();
}

void DASHPlayer::onSettingsChanged(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation)
{
    if(this->mpdWrapper->getMPD() == NULL)
        return;

    if (!this->settingsChanged(period, videoAdaptationSet, videoRepresentation, audioAdaptationSet, audioRepresentation))
        return;

    this->mpdWrapper->settingsChanged(period, videoAdaptationSet, videoRepresentation, audioAdaptationSet, audioRepresentation);
    this->multimediaManager->setVideoQuality();
}

void DASHPlayer::onVideoBufferStateChanged(uint32_t fillstateInPercent)
{
    emit videoBufferFillStateChanged(fillstateInPercent);
}

void DASHPlayer::onVideoSegmentBufferStateChanged(uint32_t fillstateInPercent)
{
    emit videoSegmentBufferFillStateChanged(fillstateInPercent);
}

void DASHPlayer::onAudioBufferStateChanged(uint32_t fillstateInPercent)
{
    emit audioBufferFillStateChanged(fillstateInPercent);
}

void DASHPlayer::onAudioSegmentBufferStateChanged(uint32_t fillstateInPercent)
{
    emit audioSegmentBufferFillStateChanged(fillstateInPercent);
}

void DASHPlayer::onEOS()
{
    this->onStopButtonPressed();
}

bool DASHPlayer::onDownloadMPDPressed (const std::string &url)
{
    this->multimediaManager->setOffset(0);
    if(this->icn)
    {
        if(!this->multimediaManager->initICN(url))
        {
            return false;
        }
    }
    else
    {
        if(!this->multimediaManager->init(url))
        {
            return false;
        }
    }
    this->setSettings(-1, -1, -1, -1, -1);
    this->gui->setMPDDuration(this->mpdWrapper->getMPD());
    return true;
}

bool DASHPlayer::settingsChanged (int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation)
{
    EnterCriticalSection(&this->monitorMutex);
    bool settingsChanged = false;
    if (this->currentSettings.videoRepresentation != videoRepresentation ||
            this->currentSettings.audioRepresentation != audioRepresentation ||
            this->currentSettings.videoAdaptationSet != videoAdaptationSet ||
            this->currentSettings.audioAdaptationSet != audioAdaptationSet ||
            this->currentSettings.period != period)
        settingsChanged = true;
    if (settingsChanged)
        this->setSettings(period, videoAdaptationSet, videoRepresentation, audioAdaptationSet, audioRepresentation);
    LeaveCriticalSection(&this->monitorMutex);
    return settingsChanged;
}

void DASHPlayer::setSettings(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation)
{
    this->currentSettings.period                = period;
    this->currentSettings.videoAdaptationSet    = videoAdaptationSet;
    this->currentSettings.videoRepresentation   = videoRepresentation;
    this->currentSettings.audioAdaptationSet    = audioAdaptationSet;
    this->currentSettings.audioRepresentation   = audioRepresentation;
}

bool DASHPlayer::downloadMPD(const QString &url, const QString &adaptationLogic, bool icn)
{

    if (this->gui->getStop())
    {
        QMetaObject::invokeMethod(this->gui->getRootObject(), "stopGraph");
        this->gui->setStop(false);
        this->gui->setPause(false);
        this->gui->setPlay(true);
        this->offset = 0;
        this->multimediaManager->setOffset(0);
        this->gui->setOffset(0);
        this->gui->setPlay(true);
        this->gui->initVideoPlayer();
        this->icn = icn;
        this->segment = 0;
        std::string mUrl;
        if(this->icn)
        {
            mUrl = this->icnPrefix + url.toStdString() + this->icnSuffix;
        }
        else
        {
            mUrl = this->httpPrefix + url.toStdString() + this->httpSuffix;
        }
        if (!this->onDownloadMPDPressed(mUrl))
            return false;

        this->segmentDuration = this->mpdWrapper->onFirstDownloadMPD(this->gui);
        this->multimediaManager->setSegmentDuration(this->segmentDuration);
        this->parametersAdaptation->segmentDuration = this->segmentDuration / 1000.0; //to have it in seconds 
        this->onSettingsChanged(0,0,0,0,0);
        int j =0;
        std::string temp = adaptationLogic.toStdString();
        temp.erase(std::remove(temp.begin(), temp.end(), ' '), temp.end());
        int adaptationLogicID = (LogicType)1; //Always Lowest by default
        for(j = 0; j < LogicType::Count; j++)
        {
            if(!strcmp(LogicType_string[j],temp.c_str()))
            {
                adaptationLogicID = (LogicType)j;
                break;
            }
        }
        if(j == LogicType::Count)
        {
            std::cout << "Could not find the ID of the adaptation logic asked, using " << LogicType_string[adaptationLogicID] << " instead.\n" << std::endl;
        }
        this->onStartButtonPressed(0,0,0,0,0, adaptationLogicID );
        this->multimediaManager->setLooping(this->repeat);
        this->adaptationLogic = adaptationLogicID;
    } else {
        if (this->gui->getPause())
        {
            this->gui->setPlay(true);
            this->gui->setPause(false);
            this->gui->getVideoPlayer()->play();
            this->gui->getVideoPlayer()->pause(false);
        }
    }
    return true;
}

void DASHPlayer::play()
{
    this->offset = 0;
    this->gui->initVideoPlayer();
    this->multimediaManager->setOffset(0);
    this->onSettingsChanged(0,0,0,0,0);
    this->onStartButtonPressed(0,0,0,0,0, this->adaptationLogic);
}

void DASHPlayer::repeatVideo(bool repeat)
{
    this->repeat = repeat;
    this->multimediaManager->setLooping(repeat);
    this->gui->setRepeat(repeat);
}

void DASHPlayer::seekVideo(float value) {
    this->multimediaManager->stop();
    this->seek = true;
    this->gui->initVideoPlayer();
    this->segment = value*this->gui->getDurationMilliseconds()/this->segmentDuration;
    this->offset = this->segment * this->segmentDuration;
    this->gui->seekSegment(this->segment);
    this->multimediaManager->setOffset(this->offset);
    if (!(this->multimediaManager->setVideoAdaptationLogic((LogicType)this->adaptationLogic, this->parametersAdaptation)))
    {
        return;
    }
    this->multimediaManager->start(this->icn, 20, this->segment);
    this->multimediaManager->setLooping(this->repeat);

}

void DASHPlayer::notifyQualityDownloading(uint32_t quality)
{
    this->qualityDownloading = quality/1000000;
}

//WARNING FPS IS USED AS BUFFERLVL
void DASHPlayer::notifyStatistics(int segNum, uint32_t bitrate, int fps, uint32_t quality)
{
    if(quality == 240)
        this->mStats[segNum] = std::make_tuple(bitrate, fps, 1);
    else if(quality == 360)
        this->mStats[segNum] = std::make_tuple(bitrate, fps, 3);
    else if(quality == 720)
        this->mStats[segNum] = std::make_tuple(bitrate, fps, 5);
    else if(quality == 1080)
        this->mStats[segNum] = std::make_tuple(bitrate, fps, 7);
    else if(quality == 1440)
        this->mStats[segNum] = std::make_tuple(bitrate, fps, 9);
    else
        this->mStats[segNum] = std::make_tuple(bitrate, fps, 11);
}

void DASHPlayer::updateSlider(qint64 value)
{
    this->position = this->offset + (uint64_t)value;
    if (this->position <= this->gui->getDurationMilliseconds()){
        this->segment = (this->offset + value)/this->segmentDuration;
        this->gui->setAnaliticsValues(std::get<0>(this->mStats[segment])/1000000,
                                      std::get<2>(this->mStats[segment]),
                                      (uint32_t)this->qualityDownloading,
                                      (double)this->multimediaManager->getBufferLevel());
        this->gui->getProgressBar()->setProperty("value", 1.0*(this->position)/(1.0*this->gui->getDurationMilliseconds()));
        this->gui->getNowLabel()->setProperty("text", QVariant(msec2string(this->position).c_str()));
        this->gui->pauseIfBuffering(this->offset + value);
    }
}

void DASHPlayer::initSlider()
{
    this->offset = 0;
    this->gui->getProgressBar()->setProperty("value", 0.0);
    this->gui->getNowLabel()->setProperty("text", QVariant("00:00:00"));
    this->gui->getLifeLabel()->setProperty("text", QVariant("00:00:00"));

}

std::string DASHPlayer::msec2string(uint64_t milliseconds)
{
    uint64_t seconds = milliseconds/1000;
    int32_t sec = seconds%60;
    seconds = (seconds - sec)/60;
    int32_t min = seconds%60;
    int32_t hours = (seconds - min)/60;
    char timeStamp[10];
    sprintf(timeStamp, "%02d:%02d:%02d", hours, min, sec);
    return std::string(timeStamp);
}

void DASHPlayer::onStopped()
{
    int posPlayer = this->position;
    if (this->seek)
    {
        this->seek = false;
    }
    else
    {
        if(!this->gui->getStop())
        {
            if (posPlayer <= 1000 || posPlayer > this->gui->getDurationMilliseconds() || (this->gui->getDurationMilliseconds()- posPlayer) <= 2000 )
            {
                if (this->repeat)
                {
                    this->gui->getStreamBuffer()->readFromNextBuffer();
                    this->gui->startIfRepeat();
                    this->offset = 0;
                    this->multimediaManager->setOffset(0);
                }
                else
                {
                    this->gui->initVideoPlayer();
                    this->gui->seekSegment(0);
                    this->gui->setStop(true);
                    this->gui->setPlay(false);
                    this->gui->setPause(false);
                    this->multimediaManager->stop();
                    QMetaObject::invokeMethod(this->gui->getRootObject(), "pauseGraph");
                    this->initSlider();
                    QMetaObject::invokeMethod(this->gui->getRootObject(), "setStop");
                }

            } else {
                qDebug("wrong position");
            }
        }
        else
        {
            this->gui->setStop(true);
            this->gui->setPlay(false);
            this->gui->setPause(false);
            this->gui->initVideoPlayer();
            this->gui->seekSegment(0);
            this->multimediaManager->stop();
            QMetaObject::invokeMethod(this->gui->getRootObject(), "pauseGraph");
            this->initSlider();
        }
    }
}

void DASHPlayer::pause()
{
    this->gui->setPlay(false);
    this->gui->setPause(true);
    this->gui->getVideoPlayer()->pause(true);
}

void DASHPlayer::setConfig (Config *config)
{
    this->config = config;
}

void DASHPlayer::manageGraph(QtAV::AVPlayer::State value)
{
    switch (value)
    {
    case QtAV::AVPlayer::State::PlayingState:
        if (config->graph())
        {
            QMetaObject::invokeMethod(this->gui->getRootObject(), "startGraph");
        }
        break;
    case QtAV::AVPlayer::State::StoppedState:
        if (config->graph())
        {
            QMetaObject::invokeMethod(this->gui->getRootObject(), "pauseGraph");
        }
        break;
    case QtAV::AVPlayer::State::PausedState:
        if (config->graph()) {
            QMetaObject::invokeMethod(this->gui->getRootObject(), "pauseGraph");
        }
        break;
    }
}

void DASHPlayer::reloadParameters()
{
    this->beta = config->beta();
    this->drop = config->drop();
    this->icnPrefix = config->icnPrefix().toStdString();
    this->httpPrefix = config->httpPrefix().toStdString();
    this->icnSuffix = config->icnSuffix().toStdString();
    this->httpSuffix = config->httpSuffix().toStdString();
    this->alpha = config->alpha();
    this->repeat = config->repeat();
    this->parametersAdaptation = (struct AdaptationParameters *)malloc(sizeof(struct AdaptationParameters));
    this->parametersAdaptation->segmentBufferSize = config->segmentBufferSize();
    this->parametersAdaptation->segmentDuration = 2;
    this->parametersAdaptation->Rate_Alpha = config->rateAlpha();
    this->parametersAdaptation->Bola_Alpha = config->bolaAlpha();
    this->parametersAdaptation->Bola_bufferTargetSeconds = config->bolaBufferTarget();
    this->parametersAdaptation->BufferBased_reservoirThreshold = config->bufferReservoirThreshold();
    this->parametersAdaptation->BufferBased_maxThreshold = config->bufferMaxThreshold();
    this->parametersAdaptation->Adaptech_Alpha = config->adaptechAlpha();
    this->parametersAdaptation->Adaptech_FirstThreshold = config->adaptechFirstThreshold();
    this->parametersAdaptation->Adaptech_SecondThreshold = config->adaptechSecondThreshold();
    this->parametersAdaptation->Adaptech_SwitchUpThreshold = config->adaptechSwitchUpMargin();
    this->parametersAdaptation->Adaptech_SlackParameter = config->adaptechSlackParameter();
    this->parametersAdaptation->BufferThreeThreshold_FirstThreshold = config->bufferThreeThresholdFirst();
    this->parametersAdaptation->BufferThreeThreshold_SecondThreshold = config->bufferThreeThresholdSecond();
    this->parametersAdaptation->BufferThreeThreshold_ThirdThreshold = config->bufferThreeThresholdThird();
    this->parametersAdaptation->Panda_Alpha = config->pandaParamAlpha();
    this->parametersAdaptation->Panda_Beta = config->pandaParamBeta();
    this->parametersAdaptation->Panda_Bmin = config->pandaParamBMin();
    this->parametersAdaptation->Panda_K = config->pandaParamK();
    this->parametersAdaptation->Panda_W = config->pandaParamW();
    this->parametersAdaptation->Panda_Epsilon = config->pandaParamEpsilon();
}

QString DASHPlayer::getLastPlayed()
{
    return config->lastPlayed();
}

void DASHPlayer::setLastPlayed(QString lastPlayed)
{
    config->setLastPlayed(lastPlayed);
}

QString DASHPlayer::getAdaptationLogic()
{
    return config->adaptationLogic();
}

void DASHPlayer::setAdaptationLogic(QString adaptationLogic)
{
    config->setAdaptationLogic(adaptationLogic);
}

bool DASHPlayer::getIcn()
{
    return config->icn();
}

void DASHPlayer::setIcn(bool icn)
{
    config->setIcn(icn);
}

QString DASHPlayer::getIcnPrefix()
{
    return config->icnPrefix();
}

void DASHPlayer::setIcnPrefix(QString icnPrefix)
{
    config->setIcnPrefix(icnPrefix);
}

QString DASHPlayer::getHttpPrefix()
{
    return config->httpPrefix();
}

void DASHPlayer::setHttpPrefix(QString httpPrefix)
{
    config->setHttpPrefix(httpPrefix);
}

QString DASHPlayer::getIcnSuffix()
{
    return config->icnSuffix();
}

void DASHPlayer::setIcnSuffix(QString icnSuffix)
{
    config->setIcnSuffix(icnSuffix);
}

QString DASHPlayer::getHttpSuffix()
{
    return config->httpSuffix();
}

void DASHPlayer::setHttpSuffix(QString httpSuffix)
{
    config->setHttpSuffix(httpSuffix);
}

qreal DASHPlayer::getAlpha()
{
    return config->alpha();
}

void DASHPlayer::setAlpha(qreal alpha)
{
    config->setAlpha(alpha);
}

qreal DASHPlayer::getSegmentBufferSize()
{
    return config->segmentBufferSize();
}

void DASHPlayer::setSegmentBufferSize(qreal segmentBufferSize)
{
    config->setSegmentBufferSize(segmentBufferSize);
}

qreal DASHPlayer::getRateAlpha()
{
    return config->rateAlpha();
}

void DASHPlayer::setRateAlpha(qreal rateAlpha)
{
    config->setRateAlpha(rateAlpha);
}

qreal DASHPlayer::getBufferReservoirThreshold()
{
    return config->bufferReservoirThreshold();
}

void DASHPlayer::setBufferReservoirThreshold(qreal bufferReservoirThreshold)
{
    config->setBufferReservoirThreshold(bufferReservoirThreshold);
}

qreal DASHPlayer::getBufferMaxThreshold()
{
    return config->bufferMaxThreshold();
}

void DASHPlayer::setBufferMaxThreshold(qreal bufferMaxThreshold)
{
    config->setBufferMaxThreshold(bufferMaxThreshold);
}

qreal DASHPlayer::getAdaptechFirstThreshold()
{
    return config->adaptechFirstThreshold();
}

void DASHPlayer::setAdaptechFirstThreshold(qreal adaptechFirstThreshold)
{
    config->setAdaptechFirstThreshold(adaptechFirstThreshold);
}

qreal DASHPlayer::getAdaptechSecondThreshold()
{
    return config->adaptechSecondThreshold();
}

void DASHPlayer::setAdaptechSecondThreshold(qreal adaptechSecondThreshold)
{
    config->setAdaptechSecondThreshold(adaptechSecondThreshold);
}

qreal DASHPlayer::getAdaptechSwitchUpMargin()
{
    return config->adaptechSwitchUpMargin();
}

void DASHPlayer::setAdaptechSwitchUpMargin(qreal adaptechSwitchUpMargin)
{
    config->setAdaptechSwitchUpMargin(adaptechSwitchUpMargin);
}

qreal DASHPlayer::getAdaptechSlackParameter()
{
    return config->adaptechSlackParameter();
}

void DASHPlayer::setAdaptechSlackParameter(qreal adaptechSlackParameter)
{
    config->setAdaptechSlackParameter(adaptechSlackParameter);
}

qreal DASHPlayer::getAdaptechAlpha()
{
    return config->adaptechAlpha();
}

void DASHPlayer::setAdaptechAlpha(qreal adaptechAlpha)
{
    config->setAdaptechAlpha(adaptechAlpha);
}

qreal DASHPlayer::getBufferThreeThresholdFirst()
{
    return config->bufferThreeThresholdFirst();
}

void DASHPlayer::setBufferThreeThresholdFirst(qreal bufferThreeThresholdFirst)
{
    config->setBufferThreeThresholdFirst(bufferThreeThresholdFirst);
}

qreal DASHPlayer::getBufferThreeThresholdSecond()
{
    return config->bufferThreeThresholdSecond();
}

void DASHPlayer::setBufferThreeThresholdSecond(qreal bufferThreeThresholdSecond)
{
    config->setBufferThreeThresholdSecond(bufferThreeThresholdSecond);
}

qreal DASHPlayer::getBufferThreeThresholdThird()
{
    return config->bufferThreeThresholdThird();
}

void DASHPlayer::setBufferThreeThresholdThird(qreal bufferThreeThresholdThird)
{
    config->setBufferThreeThresholdThird(bufferThreeThresholdThird);
}

qreal DASHPlayer::getPandaParamAlpha()
{
    return config->pandaParamAlpha();
}

void DASHPlayer::setPandaParamAlpha(qreal pandaParamAlpha)
{
    config->setPandaParamAlpha(pandaParamAlpha);
}

qreal DASHPlayer::getPandaParamBeta()
{
    return config->pandaParamBeta();
}

void DASHPlayer::setPandaParamBeta(qreal pandaParamBeta)
{
    config->setPandaParamBeta(pandaParamBeta);
}

qreal DASHPlayer::getPandaParamBMin()
{
    return config->pandaParamBMin();
}

void DASHPlayer::setPandaParamBMin(qreal pandaParamBMin)
{
    config->setPandaParamBMin(pandaParamBMin);
}

qreal DASHPlayer::getPandaParamK()
{
    return config->pandaParamK();
}

void DASHPlayer::setPandaParamK(qreal pandaParamK)
{
    config->setPandaParamK(pandaParamK);
}

qreal DASHPlayer::getPandaParamW()
{
    return config->pandaParamW();
}

void DASHPlayer::setPandaParamW(qreal pandaParamW)
{
    config->setPandaParamW(pandaParamW);
}

qreal DASHPlayer::getPandaParamEpsilon()
{
    return config->pandaParamEpsilon();
}

void DASHPlayer::setPandaParamEpsilon(qreal pandaParamEpsilon)
{
    config->setPandaParamEpsilon(pandaParamEpsilon);
}

qreal DASHPlayer::getBolaBufferTarget()
{
    return config->bolaBufferTarget();
}

void DASHPlayer::setBolaBufferTarget(qreal bolaBufferTarget)
{
    config->setBolaBufferTarget(bolaBufferTarget);
}

qreal DASHPlayer::getBolaAlpha()
{
    return config->bolaAlpha();
}

void DASHPlayer::setBolaAlpha(qreal bolaAlpha)
{
    config->setBolaAlpha(bolaAlpha);
}

bool DASHPlayer::getRepeat()
{
    return config->repeat();
}

void DASHPlayer::setRepeat(bool repeat)
{
    this->repeatVideo(repeat);
    config->setRepeat(repeat);
}

bool DASHPlayer::getGraph()
{
    return config->graph();
}

void DASHPlayer::setGraph (bool graph)
{
    config->setGraph(graph);
    if (graph) {
        if (this->gui->getPlay() && this->gui->getVideoPlayer()->isPlaying()) {
            QMetaObject::invokeMethod(this->gui->getRootObject(), "startGraph");
        }
    } else {
        QMetaObject::invokeMethod(this->gui->getRootObject(), "stopGraph");
    }
}

bool DASHPlayer::getFullScreen()
{
    return config->fullScreen();
}

void DASHPlayer::setFullScreen(bool fullScreen)
{
    config->setFullScreen(fullScreen);
}

bool DASHPlayer::getStop()
{
    return this->gui->getStop();
}

bool DASHPlayer::getAutotune()
{
    return config->autotune();
}

void DASHPlayer::setAutotune(bool autotune)
{
    config->setAutotune(autotune);
}

int DASHPlayer::getLifetime()
{
    return config->lifetime();
}

void DASHPlayer::setLifetime(int lifetime)
{
    config->setLifetime(lifetime);
}

int DASHPlayer::getRetransmissions()
{
    return config->retransmissions();
}

void DASHPlayer::setRetransmissions(int retransmissions)
{
    config->setRetransmissions(retransmissions);
}

qreal DASHPlayer::getBeta()
{
    return config->beta();
}

void DASHPlayer::setBeta(qreal beta)
{
    config->setBeta(beta);
    this->multimediaManager->setBeta(beta);
}

qreal DASHPlayer::getDrop()
{
    return config->drop();
}

void DASHPlayer::setDrop(qreal drop)
{
    config->setDrop(drop);
    this->multimediaManager->setDrop(drop);
}

qreal DASHPlayer::getBetaWifi()
{
    return config->betaWifi();
}

void DASHPlayer::setBetaWifi(qreal betaWifi)
{
    config->setBetaWifi(betaWifi);
}

qreal DASHPlayer::getDropWifi()
{
    return config->dropWifi();
}

void DASHPlayer::setDropWifi(qreal dropWifi)
{
    config->setDropWifi(dropWifi);
}

int DASHPlayer::getDelayWifi()
{
    return config->delayWifi();
}

void DASHPlayer::setDelayWifi(int delayWifi)
{
    config->setDelayWifi(delayWifi);
}

qreal DASHPlayer::getBetaLte()
{
    return config->betaLte();
}

void DASHPlayer::setBetaLte(qreal betaLte)
{
    config->setBetaLte(betaLte);
}

qreal DASHPlayer::getDropLte()
{
    return config->dropLte();
}

void DASHPlayer::setDropLte(qreal dropLte)
{
    config->setDropLte(dropLte);
}

int DASHPlayer::getDelayLte()
{
    return config->delayLte();
}

void DASHPlayer::setDelayLte(int delayLte)
{
    config->setDelayLte(delayLte);
}

int DASHPlayer::getBatchingParameter()
{
    return config->batchingParameter();
}

void DASHPlayer::setBatchingParameter(int batchingParameter)
{
    config->setBatchingParameter(batchingParameter);
}

int DASHPlayer::getRateEstimator ()
{
    return config->rateEstimator();
}

void DASHPlayer::setRateEstimator(int rateEstimator)
{
    config->setRateEstimator(rateEstimator);
}

void DASHPlayer::error(const QtAV::AVError &e)
{
    qDebug("error in the player!");
    seekVideo(0);
}



