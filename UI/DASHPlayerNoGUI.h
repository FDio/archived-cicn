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

#ifndef DASHPLAYERNOGUI_H_
#define DASHPLAYERNOGUI_H_

#include <iostream>
#include <sstream>
#include "libdash.h"
#include "DASHPlayer.h"
#include "IDASHPlayerNoGuiObserver.h"
#include "../Managers/IMultimediaManagerObserver.h"
#include "../Managers/MultimediaManager.h"
#include "../Adaptation/IAdaptationLogic.h"
#include "../Buffer/IBufferObserver.h"
#include "../MPD/AdaptationSetHelper.h"
#include "UI/GraphDataSource.h"
#include "Websocket/WebSocketService.h"

namespace viper
{
class DASHPlayerNoGUI : public IDASHPlayerNoGuiObserver, public managers::IMultimediaManagerObserver
{
public:
    DASHPlayerNoGUI (int argc, char** argv, pthread_cond_t *mainCond, bool nodecoding);
    virtual ~DASHPlayerNoGUI ();
    void parseArgs(int argc, char ** argv);
    void helpMessage(char *name);
    virtual void onStartButtonPressed(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation);
    virtual void onStopButtonPressed();
    virtual void onSettingsChanged(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation);
    /* IMultimediaManagerObserver */
    virtual void onVideoBufferStateChanged(uint32_t fillstateInPercent);
    virtual void onVideoSegmentBufferStateChanged(uint32_t fillstateInPercent);
    virtual void onAudioBufferStateChanged(uint32_t fillstateInPercent);
    virtual void onAudioSegmentBufferStateChanged(uint32_t fillstateInPercent);
    virtual void onEOS();
    virtual void notifyStatistics(int, uint32_t, int, uint32_t);
    virtual void notifyQualityDownloading(uint32_t);
    virtual bool onDownloadMPDPressed(const std::string &url);
    bool isRunning();

private:
    dash::mpd::IMPD			*mpd;
    viper::managers::MultimediaManager	*multimediaManager;
    CRITICAL_SECTION		monitorMutex;
    char					*url;
    bool					isICN;
    libdash::framework::adaptation::LogicType	adaptLogic;
    pthread_cond_t			*mainCond;
    bool					running;
    struct libdash::framework::adaptation::AdaptationParameters *parameterAdaptation;
    float	segmentDuration;
    int		segmentBufferSize;
    double	alpha;
    double	rateAlpha;
    double	bolaAlpha;
    double	bolaBufferTargetSeconds;
    int		bufferBasedReservoirThreshold;
    int		bufferBasedMaxThreshold;
    double	adaptechAlpha;
    int		adaptechFirstThreshold;
    int		adaptechSecondThreshold;
    int		adaptechSwitchUpThreshold;
    int		bufferThreeThreshold_FirstThreshold;
    int		bufferThreeThreshold_SecondThreshold;
    int		bufferThreeThreshold_ThirdThreshold;
    double	pandaAlpha;
    double	pandaParam_Beta;
    double	pandaParam_Bmin;
    double	pandaParam_K;
    double	pandaParam_W;
    double	pandaParamEpsilon;
    bool	repeat;
    GraphDataSource *graphData;
    bool noDecoding;
    bool settingsChanged(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation);

};
}
#endif /* DASHPLAYER_H_ */
