/*
 * DASHManager.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_INPUT_DASHMANAGER_H_
#define LIBDASH_FRAMEWORK_INPUT_DASHMANAGER_H_

#include "DASHReceiver.h"
#include "IDASHReceiverObserver.h"
#include "libdash.h"
#include "IMPD.h"
#include <QtMultimedia/qaudioformat.h>
#include "IDASHManagerObserver.h"
//#include "../Buffer/Segment.h"
#include "../Managers/IStreamObserver.h"
#include "../Buffer/IBufferObserver.h"

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

class DASHManager : public IDASHReceiverObserver, public IBufferObserver
{
public:
    DASHManager             (viper::managers::StreamType type, uint32_t maxCapacity, IDASHManagerObserver *multimediaStream, libdash::framework::mpd::MPDWrapper *mpdWrapper, bool icnEnabled, double icnAlpha, bool nodecoding, float beta, float drop, std::string v6FirstWord);
    virtual ~DASHManager    ();

    bool start();
    void stop();
    uint32_t getPosition();
    void setPosition(uint32_t segmentNumber);
    void setLooping(bool looping);
    void setPositionInMsec(uint32_t millisec);
    void clear();
    void setRepresentation();
    void enqueueRepresentation(dash::mpd::IPeriod *period, dash::mpd::IAdaptationSet *adaptationSet, dash::mpd::IRepresentation *representation);

    void onSegmentDownloaded();
    void notifyStatistics(int, uint32_t, int, uint32_t);
    void notifyQualityDownloading (uint32_t);
    bool canPush();
    int	getBufferLevel();
    void setAdaptationLogic(libdash::framework::adaptation::IAdaptationLogic *_adaptationLogic);
    bool isICN();
    void shouldAbort();

    void setTargetDownloadingTime(double);
    MediaObject* getSegment();
    void onBufferStateChanged(BufferType type, uint32_t fillstateInPercent, int maxC);
    void fetchMPD();

private:
    float                                               beta;
    float                                               drop;
    std::string                                         v6FirstWord;
    buffer::Buffer<MediaObject>                         *buffer;
    DASHReceiver                                        *receiver;
    uint32_t                                            readSegmentCount;
    IDASHManagerObserver                                *multimediaStream;
    bool                                                isRunning;
    bool                                                icn;
    double                                              icnAlpha;
    bool                                                noDecoding;
    libdash::framework::adaptation::IAdaptationLogic    *adaptationLogic;
};
}
}
}

#endif /* LIBDASH_FRAMEWORK_INPUT_DASHMANAGER_H_ */
