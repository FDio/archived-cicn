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

#include "DASHPlayerNoGUI.h"
#include <iostream>

using namespace libdash::framework::adaptation;
using namespace libdash::framework::mpd;
using namespace libdash::framework::buffer;
using namespace viper;
using namespace viper::managers;
using namespace dash::mpd;
using namespace std;

DASHPlayerNoGUI::DASHPlayerNoGUI(int argc, char ** argv, pthread_cond_t *mainCond, bool nodecoding) :
    mainCond	(mainCond),
    running	(true),
    noDecoding	(nodecoding)
{
    InitializeCriticalSection(&this->monitorMutex);
    this->url = NULL;
    this->adaptLogic = LogicType::RateBased;
    this->isICN = false;
    this->alpha = -1;
    this->graphData = NULL;
    this->parameterAdaptation = (struct libdash::framework::adaptation::AdaptationParameters *)malloc(sizeof(struct libdash::framework::adaptation::AdaptationParameters));

    this->parameterAdaptation->segmentDuration = 2.0;
    this->parameterAdaptation->segmentBufferSize = 10;

    this->parameterAdaptation->Rate_Alpha = 0.8;

    this->parameterAdaptation->Bola_Alpha = 0.8;
    this->parameterAdaptation->Bola_bufferTargetSeconds = 8.0;

    this->parameterAdaptation->BufferBased_reservoirThreshold = 25;
    this->parameterAdaptation->BufferBased_maxThreshold = 75;

    this->parameterAdaptation->Adaptech_Alpha = 0.8;
    this->parameterAdaptation->Adaptech_FirstThreshold = 25;
    this->parameterAdaptation->Adaptech_SecondThreshold = 45;
    this->parameterAdaptation->Adaptech_SwitchUpThreshold = 5;
    this->parameterAdaptation->Adaptech_SlackParameter = 0.8;

    this->parameterAdaptation->BufferThreeThreshold_FirstThreshold = 25;
    this->parameterAdaptation->BufferThreeThreshold_SecondThreshold = 50;
    this->parameterAdaptation->BufferThreeThreshold_ThirdThreshold = 75;

    this->parameterAdaptation->Panda_Alpha = 0.2;
    this->parameterAdaptation->Panda_Beta = 0.2;
    this->parameterAdaptation->Panda_Bmin = 44;
    this->parameterAdaptation->Panda_K = 0.14;
    this->parameterAdaptation->Panda_W = 300000;
    this->parameterAdaptation->Panda_Epsilon = 0.15;

    this->repeat = false;
    this->parseArgs(argc, argv);

    this->multimediaManager = new MultimediaManager(NULL, this->parameterAdaptation->segmentBufferSize, "/tmp/",  noDecoding);
    this->mpdWrapper = new MPDWrapper(NULL);
    this->multimediaManager->setMPDWrapper(this->mpdWrapper);
    this->multimediaManager->attachManagerObserver(this);

    if(this->url == NULL)
    {
        this->running = false;
        pthread_cond_broadcast(mainCond);
        return;
    }
    else
    {
        if(this->onDownloadMPDPressed(string(this->url).c_str()))
        {
            this->graphData = new GraphDataSource(NULL);
            WebSocketService webSocketService;
            webSocketService.setGraphDataSource(this->graphData);
            webSocketService.start();
            this->parameterAdaptation->segmentDuration = this->mpdWrapper->onFirstDownloadMPD(NULL);
            this->multimediaManager->setSegmentDuration(this->parameterAdaptation->segmentDuration);
            //should be in seconds
            this->parameterAdaptation->segmentDuration = this->parameterAdaptation->segmentDuration / 1000.0;
            this->onStartButtonPressed(0,0,0,0,0);
            this->multimediaManager->setLooping(this->repeat);
        }
        else
        {
            this->running = false;
            pthread_cond_broadcast(mainCond);
        }
    }
}

DASHPlayerNoGUI::~DASHPlayerNoGUI()
{
    this->multimediaManager->stop();
    delete(this->multimediaManager);
    if(this->mpdWrapper)
        delete(this->mpdWrapper);
    if(this->graphData)
        delete(this->graphData);
    DeleteCriticalSection(&this->monitorMutex);
}

void DASHPlayerNoGUI::setMPDWrapper(MPDWrapper *mpdWrapper)
{
    this->mpdWrapper = mpdWrapper;
}

void DASHPlayerNoGUI::onStartButtonPressed(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation)
{
    this->onSettingsChanged(period,videoAdaptationSet,videoRepresentation, audioAdaptationSet, audioRepresentation);
    bool setOk = false;
    setOk = this->multimediaManager->setVideoAdaptationLogic((LogicType)adaptLogic, this->parameterAdaptation);

    if(!setOk)
    {
        return;
    }

    Debug("DASH PLAYER:	STARTING VIDEO\n");
    this->multimediaManager->start(this->isICN, this->alpha, 0);
}

void DASHPlayerNoGUI::onStopButtonPressed                ()
{
    this->running = false;
    pthread_cond_broadcast(mainCond);
}

bool DASHPlayerNoGUI::isRunning							 ()
{
    return this->running;
}

void DASHPlayerNoGUI::notifyStatistics(int segNum, uint32_t bitrate, int fps, uint32_t quality)
{
}

void DASHPlayerNoGUI::notifyQualityDownloading(uint32_t quality)
{
    this->graphData->setAnaliticsValues(quality/1000000, 0, quality/1000000, 0);
}

void DASHPlayerNoGUI::parseArgs(int argc, char ** argv)
{
    if(argc == 1)
    {
        helpMessage(argv[0]);
        return;
    }
    else
    {
        int i = 0;
        while(i < argc)
        {
            if(!strcmp(argv[i],"-u"))
            {
                this->url = argv[i+1];
                i++;
                i++;
                continue;
            }
            if(!strcmp(argv[i],"-n"))
            {
                this->isICN = true;
                this->alpha = -1;
                i++;
                continue;
            }
            if(!strcmp(argv[i],"-loop"))
            {
                this->repeat = true;
                i++;
                continue;
            }
            if(!strcmp(argv[i],"-nr"))
            {
                this->isICN = true;
                this->alpha = atof(argv[i+1]);
                i++;
                i++;
                continue;
            }
            if(!strcmp(argv[i], "-b"))
            {
                this->adaptLogic = LogicType::BufferBased;
                this->parameterAdaptation->BufferBased_reservoirThreshold = atoi(argv[i+1]);
                this->parameterAdaptation->BufferBased_maxThreshold = atoi(argv[i+2]);
                i = i + 3;
                continue;
            }
            if(!strcmp(argv[i], "-br"))
            {
                this->adaptLogic = LogicType::BufferRateBased;
                this->parameterAdaptation->Adaptech_Alpha = atof(argv[i+1]);
                this->parameterAdaptation->Adaptech_FirstThreshold = atoi(argv[i+2]);
                this->parameterAdaptation->Adaptech_SecondThreshold = atoi(argv[i+3]);
                this->parameterAdaptation->Adaptech_SwitchUpThreshold = atoi(argv[i+4]);
                i = i + 4;
                continue;
            }
            if(!strcmp(argv[i], "-bola"))
            {
                this->adaptLogic = LogicType::Bola;
                this->parameterAdaptation->Bola_Alpha = atof(argv[i+1]);
                this->parameterAdaptation->Bola_bufferTargetSeconds = atoi(argv[i+2]);
                i = i + 2;
                continue;
            }
            if(!strcmp(argv[i], "-bt"))
            {
                this->adaptLogic = LogicType::BufferBasedThreeThreshold;
                this->parameterAdaptation->BufferThreeThreshold_FirstThreshold = atoi(argv[i+1]);
                this->parameterAdaptation->BufferThreeThreshold_SecondThreshold = atoi(argv[i+2]);
                this->parameterAdaptation->BufferThreeThreshold_ThirdThreshold = atoi(argv[i+3]);
                i = i + 3;
                continue;
            }
            if(!strcmp(argv[i], "-r"))
            {
                this->adaptLogic = LogicType::RateBased;
                this->parameterAdaptation->Rate_Alpha = atof(argv[i+1]);
                i = i + 2;
                continue;
            }
            if(!strcmp(argv[i], "-p"))
            {
                this->adaptLogic = LogicType::Panda;
                this->parameterAdaptation->Panda_Alpha = atof(argv[i+1]);
                i = i + 2;
                continue;
            }

            if(!strcmp(argv[i],"-a"))
            {
                int j =0;
                for(j = 0; j < LogicType::Count; j++)
                {
                    if(!strcmp(LogicType_string[j],argv[i+1]))
                    {
                        this->adaptLogic = (LogicType)j;
                        break;
                    }
                }
                if(j == LogicType::Count)
                {
                    std::cout << "the different adaptation logics implemented are:" << std::endl;
                    for(j = 0;j < LogicType::Count; j++)
                    {
                        std::cout << LogicType_string[j] << std::endl;
                    }
                    std::cout << "By default, the " << LogicType_string[this->adaptLogic] << " logic is selected." << std::endl;
                }
                i++;
                i++;
                continue;
            }
            i++;
        }
    }
}

void DASHPlayerNoGUI::helpMessage(char * name)
{
    std::cout << "Usage: " << name << " -u url -a adaptationLogic -n" << std::endl << \
                 "-u:\tThe MPD's url" << std::endl << \
                 "-a:\tThe adaptationLogic:" << std::endl << \
                 "\t*AlwaysLowest" << std::endl << \
                 "\t*RateBased(default)" << std::endl << \
                 "\t*BufferBased" << std::endl << \
                 "-n:\tFlag to use ICN instead of TCP" << std::endl << \
                 "-nr alpha:\tFlag to use ICN instead of TCP and estimation at packet lvl" << std::endl << \
                 "-b reservoirThreshold maxThreshold (both in %)" << std::endl << \
                 "-br alpha reservoirThreshold maxThreshold" << std::endl << \
                 "-r alpha" << std::endl;
}

void DASHPlayerNoGUI::onSettingsChanged(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation)
{
    if(this->mpdWrapper->getMPD() == NULL)
        return;

    if (!this->settingsChanged(period, videoAdaptationSet, videoRepresentation, audioAdaptationSet, audioRepresentation))
        return;

//    IPeriod                         *currentPeriod      = this->multimediaManager->getMPD()->GetPeriods().at(period);
//    std::vector<IAdaptationSet *>   videoAdaptationSets = AdaptationSetHelper::getVideoAdaptationSets(currentPeriod);
//    std::vector<IAdaptationSet *>   audioAdaptationSets = AdaptationSetHelper::getAudioAdaptationSets(currentPeriod);
//
//    if (videoAdaptationSet >= 0 && videoRepresentation >= 0 && !videoAdaptationSets.empty())
//    {
//        this->multimediaManager->setVideoQuality(currentPeriod,
//                                                 videoAdaptationSets.at(videoAdaptationSet),
//                                                 videoAdaptationSets.at(videoAdaptationSet)->GetRepresentation().at(videoRepresentation));
//    }
//    else
//    {
//        this->multimediaManager->setVideoQuality(currentPeriod, NULL, NULL);
//    }
//
//    if (audioAdaptationSet >= 0 && audioRepresentation >= 0 && !audioAdaptationSets.empty())
//    {
//        this->multimediaManager->setAudioQuality(currentPeriod,
//                                                 audioAdaptationSets.at(audioAdaptationSet),
//                                                 audioAdaptationSets.at(audioAdaptationSet)->GetRepresentation().at(audioRepresentation));
//    }
//    else
//    {
//        this->multimediaManager->setAudioQuality(currentPeriod, NULL, NULL);
//    }
    this->mpdWrapper->settingsChanged(period, videoAdaptationSet, videoRepresentation, audioAdaptationSet, audioRepresentation);
    this->multimediaManager->setVideoQuality();
}

void DASHPlayerNoGUI::onEOS()
{
    this->onStopButtonPressed();
}

bool DASHPlayerNoGUI::onDownloadMPDPressed(const std::string &url)
{
    if(this->isICN)
    {
        if(!this->multimediaManager->initICN(url))
        {
            std::cout << "Problem parsing the mpd. ICN is enabled." << std::endl;
            return false;
        }
    }
    else
    {
        if(!this->multimediaManager->init(url))
        {
            std::cout << "Problem parsing the mpd. ICN is disabled." << std::endl;
            return false;
        }
    }
    return true;
}

bool DASHPlayerNoGUI::settingsChanged(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation)
{
    return true;
}

void DASHPlayerNoGUI::onVideoBufferStateChanged(uint32_t fillstateInPercent)
{
}

void DASHPlayerNoGUI::onVideoSegmentBufferStateChanged(uint32_t fillstateInPercent)
{
}

void DASHPlayerNoGUI::onAudioBufferStateChanged(uint32_t fillstateInPercent)
{
}

void DASHPlayerNoGUI::onAudioSegmentBufferStateChanged(uint32_t fillstateInPercent)
{
}
