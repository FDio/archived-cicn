/*
 * MultimediaStream.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef VIPER_MANAGERS_MULTIMEDIASTREAM_H_
#define VIPER_MANAGERS_MULTIMEDIASTREAM_H_

#include "IMPD.h"
#include "IStreamObserver.h"
#include "../MPD/MPDWrapper.h"
#include "../Input/DASHManager.h"
#include "../Buffer/IBufferObserver.h"
#include "../Adaptation/IAdaptationLogic.h"
#include <QtMultimedia/qaudioformat.h>
#include "../Input/IDASHManagerObserver.h"
#include "../Buffer/Buffer.h"
#include <QImage>

namespace libdash
{
namespace framework
{
namespace mpd
{
class MPDWrapper;
}
namespace input
{
class DASHManager;
}
}
}
namespace viper
{
namespace managers
{
class MultimediaStream : public libdash::framework::input::IDASHManagerObserver, public libdash::framework::buffer::IBufferObserver
{
public:
    MultimediaStream(StreamType type, libdash::framework::mpd::MPDWrapper *mpdWrapper, uint32_t segmentBufferSize, bool icnEnabled, double icnAlpha, bool nodecoding, float beta, float drop);
    virtual ~MultimediaStream();

    bool start();
    void stop();
    void stopDownload();
    bool startDownload();
    void clear();
    uint32_t getPosition();
    void setPosition(uint32_t segmentNumber);
    void setLooping(bool looping);
    void setPositionInMsec(uint32_t milliSecs);
    libdash::framework::input::MediaObject* getSegment();
    void setEOS(bool value);
    void notifyBufferChange(uint32_t bufferfill, int maxC);
    void setRepresentation();
    void enqueueRepresentation(dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet, dash::mpd::IRepresentation *representation);
    void setAdaptationLogic(libdash::framework::adaptation::IAdaptationLogic *logic);

    void attachStreamObserver(IStreamObserver *observer);

    void onSegmentBufferStateChanged(uint32_t fillstateInPercent, int maxC);
    void onBufferStateChanged(libdash::framework::buffer::BufferType type, uint32_t fillstateInPercent, int maxC);

    void notifyStatistics(int segNum, uint32_t bitrate, int fps, uint32_t quality);
    void notifyQualityDownloading (uint32_t quality);
    bool canPush();
    int	getBufferLevel();
    bool isICN();
    void shouldAbort();
    void setTargetDownloadingTime(double);
    void fetchMPD();

private:
    float                                               beta;
    float                                               drop;
    std::vector<IStreamObserver *>                      observers;
    libdash::framework::mpd::MPDWrapper                 *mpdWrapper;
    libdash::framework::adaptation::IAdaptationLogic    *logic;
    libdash::framework::input::DASHManager              *dashManager;
    uint32_t                                            segmentBufferSize;
    StreamType                                          type;
    bool                                                icn;
    double                                              icnAlpha;
    bool                                                noDecoding;

    void init();
};
}
}

#endif /* VIPER_MANAGERS_MULTIMEDIASTREAM_H_ */
