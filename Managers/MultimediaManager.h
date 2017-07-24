/*
 * MultimediaManager.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef VIPER_MANAGERS_MULTIMEDIAMANAGER_H_
#define VIPER_MANAGERS_MULTIMEDIAMANAGER_H_

#include "IMPD.h"
#include "MultimediaStream.h"
#include "IMultimediaManagerBase.h"
#include "IMultimediaManagerObserver.h"
#include "../Adaptation/IAdaptationLogic.h"
#include "../Adaptation/AdaptationLogicFactory.h"
#include "../Portable/MultiThreading.h"
#include <QtMultimedia/qaudiooutput.h>
#include <cstring>
#include <QtAV/QtAV.h>
#include "Common/ViperBuffer.h"
#include "UI/ViperGui.h"

namespace viper
{
namespace managers
{
class MultimediaManager : public IStreamObserver, public IMultimediaManagerBase
{
public:
    MultimediaManager(ViperGui *viperGui, int segmentBufferSize, std::string downloadPath, bool noDecoding = false);
    virtual ~MultimediaManager();

    bool                                    init	                    (const std::string& url);
    bool                                    initICN	                    (const std::string& url);
    void                                    start	                    (bool icnEnabled, double icnAlpha, uint32_t nextOffset);
    void                                    stop                        ();
    dash::mpd::IMPD*                        getMPD                      ();
    bool                                    setVideoQuality             ();
    bool                                    setAudioQuality             ();
    bool                                    setVideoAdaptationLogic     (libdash::framework::adaptation::LogicType type, struct libdash::framework::adaptation::AdaptationParameters *params);
    bool                                    setAudioAdaptationLogic     (libdash::framework::adaptation::LogicType type, struct libdash::framework::adaptation::AdaptationParameters *params);
    void                                    attachManagerObserver       (IMultimediaManagerObserver *observer);
    void                                    setFrameRate                (double frameRate);
    void                                    setSegmentDuration          (float segDuration);
    float                                   getSegmentDuration          ();

    /* IStreamObserver */
    void                                    onSegmentDownloaded        	();
    void                                    onSegmentBufferStateChanged	(StreamType type, uint32_t fillstateInPercent, int maxC);
    void                                    onVideoBufferStateChanged	(uint32_t fillstateInPercent);
    void                                    onAudioBufferStateChanged 	(uint32_t fillstateInPercent);
    bool                                    isUserDependent				();
    bool                                    isStarted					();
    bool                                    isStopping					();
    bool                                    isICN						();
    void                                    setEOS						(bool value);
    void                                    shouldAbort					(bool isVideo);
    void                                    setTargetDownloadingTime    (bool isVid, double time);
    bool                                    isPlaying                   ();
    void                                    onPausePressed              ();
    void                                    notifyStatistics            (int segNum, uint32_t bitrate, int fps, uint32_t quality);
    void                                    notifyQualityDownloading    (uint32_t quality);
    uint32_t                                getUBufferLevel             ();
    int	                                    getBufferLevel              ();
    void                                    setLooping                  (bool looping);
    libdash::framework::mpd::MPDWrapper*    getMPDWrapper               ();
    void                                    setMPDWrapper               (libdash::framework::mpd::MPDWrapper* mpdWrapper);
    void                                    setOffset					(int offset);
    void                                    setBeta                     (float beta);
    void                                    setDrop                     (float drop);
    bool                                    canPush                     ();
    void                                    fetchMPD                    ();


    CRITICAL_SECTION                                        monitorBufferMutex;
    int                                                     offset;
    std::chrono::time_point<std::chrono::system_clock>		lastPointInTime;
    std::chrono::time_point<std::chrono::system_clock>		bufferingLimit;

private:
    float                                                       beta;
    float                                                       drop;
    std::string                                                 downloadPath;
    int                                                         segmentBufferSize;
    ViperGui                                                    *viperGui;
    dash::IDASHManager                                          *manager;
    libdash::framework::mpd::MPDWrapper                         *mpdWrapper;
    libdash::framework::adaptation::IAdaptationLogic            *videoLogic;
    MultimediaStream                                            *videoStream;
    libdash::framework::adaptation::IAdaptationLogic            *audioLogic;
    MultimediaStream                                            *audioStream;
    std::vector<IMultimediaManagerObserver *>                   managerObservers;
    bool                                                        started;
    bool                                                        stopping;
    bool                                                        icn;
    double                                                      icnAlpha;
    libdash::framework::input::IICNConnection                   *icnConn;
    uint64_t                                                    framesDisplayed;
    uint64_t                                                    segmentsDownloaded;
    CRITICAL_SECTION                                            monitorMutex;
    double                                                      frameRate;
    THREAD_HANDLE                                               videoRendererHandle;
    THREAD_HANDLE                                               audioRendererHandle;
    THREAD_HANDLE                                               mpdFetcherThread;
    bool                                                        isVideoRendering;
    bool                                                        isAudioRendering;
    bool                                                        eos;
    bool                                                        playing;
    mutable CRITICAL_SECTION                                    monitor_playing_video_mutex;
    mutable CONDITION_VARIABLE                                  playingVideoStatusChanged;
    mutable CRITICAL_SECTION                                	monitor_playing_audio_mutex;
    mutable CONDITION_VARIABLE                              	playingAudioStatusChanged;
    const char                                                  *logicName;
    bool                                                        noDecoding;
    std::string                                                 url;
    float                                                       segmentDuration;

    void                                                        notifyBufferChange                  ();
    bool                                                        startVideoRenderingThread           ();
    void                                                        stopVideoRenderingThread            ();
    static void*                                                pushVideo                           (void *data);
    static void*                                                pushVideoNoOut                      (void *data);
    bool                                                        startAudioRenderingThread           ();
    void                                                        stopAudioRenderingThread            ();
    void                                                        initVideoRendering                  (uint32_t offset);
    void                                                        initAudioPlayback                   (uint32_t offset);
    void                                                        stopVideo                           ();
    void                                                        stopAudio                           ();
    void                                                        notifyVideoBufferObservers          (uint32_t fillstateInPercent);
    void                                                        notifyVideoSegmentBufferObservers   (uint32_t fillstateInPercent);
    void                                                        notifyAudioBufferObservers          (uint32_t fillstateInPercent);
    void                                                        notifyAudioSegmentBufferObservers   (uint32_t fillstateInPercent);
    void                                                        updateMPD                           ();
    void                                                        updateMPDICN                        ();
    static void*                                                DoMPDFetching				        (void* manager);
};
}
}

#endif /* VIPER_MANAGERS_MULTIMEDIAMANAGER_H_ */
