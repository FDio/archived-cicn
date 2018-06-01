/*
 * MPDWrapper.h
 *****************************************************************************
 * Copyright (C) 2017, Cisco Systems France
 *
 * Email: cicn-dev@lists.fd.io
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/


#ifndef LIBDASH_FRAMEWORK_MPD_MPDWRAPPER_H_
#define LIBDASH_FRAMEWORK_MPD_MPDWRAPPER_H_
//TODO: fix the circular includes
namespace viper
{
class ViperGui;
}
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
class MediaObject;
class DASHReceiver;
}
namespace adaptation
{
class IAdaptationLogic;
}
}
}
#include "IMPD.h"
#include "IMPDWrapper.h"
#include "../Portable/MultiThreading.h"
#include "../Managers/IStreamObserver.h"
#include "../UI/IViperGui.h"
#include "IRepresentationStream.h"
#include "IRepresentation.h"
#include "IPeriod.h"
#include "IAdaptationSet.h"
#include "RepresentationStreamFactory.h"
#include "../Input/MediaObject.h"
#include "AdaptationSetHelper.h"


namespace libdash
{
namespace framework
{
namespace mpd
{
class MPDWrapper : public IMPDWrapper
{
public:
    MPDWrapper(dash::mpd::IMPD *mpd);
    ~MPDWrapper();

    dash::mpd::IMPD*                            getMPD();
    void                                        updateMPD(dash::mpd::IMPD* mpd);
    std::string                                 getType();
    void                                        reInit(viper::managers::StreamType type);
    void                                        setVideoQuality(dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, dash::mpd::IRepresentation* representation);
    void                                        setAudioQuality(dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, dash::mpd::IRepresentation* representation);
    bool                                        hasAudioAdaptationSetAndAudioRepresentation();
    bool                                        hasVideoAdaptationSetAndVideoRepresentation();
    void                                        initializeAdaptationSetStream(viper::managers::StreamType type);
//    void                                      initializeAdaptationSetStream(viper::managers::StreamType type, dash::mpd::IMPD* mpd);
    void                                        destroyAdaptationSetStream(viper::managers::StreamType type);
    void                                        acquireLock();
    void                                        releaseLock();
    void                                        setSegmentOffset(viper::managers::StreamType type, uint32_t segmentOffset);
    void                                        findVideoAdaptationSet(dash::mpd::IMPD* mpd);
    void                                        findAudioAdaptationSet(dash::mpd::IMPD* mpd);
    void                                        findVideoRepresentation(dash::mpd::IMPD* mpd);
    void                                        findAudioRepresentation(dash::mpd::IMPD* mpd);
    void                                        initializeAdaptationSetStreamWithoutLock(viper::managers::StreamType type);
    void                                        initializeAdaptationSetStreamWithoutLock(viper::managers::StreamType type, dash::mpd::IMPD* mpd);
    std::vector<dash::mpd::IBaseUrl *>          resolveBaseUrl(viper::managers::StreamType type, size_t mpdBaseUrl, size_t periodBaseUrl, size_t adaptationSetBaseUrl);
    std::vector<dash::mpd::IBaseUrl *>          resolveBaseUrl(viper::managers::StreamType type, size_t mpdBaseUrl, size_t periodBaseUrl, size_t adaptationSetBaseUrl, dash::mpd::IMPD* mpd);
    libdash::framework::input::MediaObject*     getNextSegment(viper::managers::StreamType type, bool isLooping, uint32_t &segmentNumber, bool withFeedBack);
    libdash::framework::input::MediaObject*     getSegment(viper::managers::StreamType type, uint32_t segNum);
    libdash::framework::input::MediaObject*     getInitSegment(viper::managers::StreamType type);
    void                                        setQuality(viper::managers::StreamType type, dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet *adaptationSet, dash::mpd::IRepresentation *representation);
    uint32_t                                    calculateSegmentOffset(viper::managers::StreamType type, uint32_t bufferSize);
    std::string                                 getRepresentationID(viper::managers::StreamType type);
    std::string                                 getPublishTime();
    std::string                                 getMinimumUpdatePeriod();
    std::vector<dash::mpd::IRepresentation *>   getRepresentations(viper::managers::StreamType type);
    std::string                                 getMediaPresentationDuration();
    dash::mpd::IRepresentation*                 getRepresentationAt(viper::managers::StreamType type, int index);
    void                                        setRepresentation(viper::managers::StreamType type, dash::mpd::IRepresentation* rep);
    std::string                                 getRepresentationIDWithoutLock(viper::managers::StreamType type);
    libdash::framework::input::MediaObject*     getInitSegmentWithoutLock(viper::managers::StreamType type);
    std::string                                 getAvailabilityStarttime();
    std::string                                 getTimeShiftBufferDepth();
    std::string                                 getTypeWithoutLock();
    std::string                                 getMinimumUpdatePeriodWithoutLock();
    uint32_t                                    getFetchTime();
    void                                        settingsChanged(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation);
    float                                       onFirstDownloadMPD(viper::IViperGui *gui);
    void                                        setIsStopping(bool isStopping);
    void                                        setSegmentIsSetFlag(viper::managers::StreamType type, bool flag);
    bool                                        getSegmentIsSetFlag(viper::managers::StreamType type);
    int                                         getSegmentQuality(viper::managers::StreamType type);
    void                                        setSegmentQuality(viper::managers::StreamType type, int segQuality);

private:
    RepresentationStreamType	determineRepresentationStreamType(dash::mpd::IRepresentation *representation, dash::mpd::IAdaptationSet* adaptationSet, dash::mpd::IPeriod* period);

    dash::mpd::IMPD                                                     *mpd;
    mutable CRITICAL_SECTION                                            monitorMutex;
    mutable CONDITION_VARIABLE                                          mpdUpdate;
    dash::mpd::IPeriod                                                  *period;
    dash::mpd::IAdaptationSet                                           *videoAdaptationSet;
    dash::mpd::IRepresentation                                          *videoRepresentation;
    dash::mpd::IAdaptationSet                                           *audioAdaptationSet;
    dash::mpd::IRepresentation                                          *audioRepresentation;
    std::map<dash::mpd::IRepresentation *, IRepresentationStream *>     *videoRepresentations;
    std::map<dash::mpd::IRepresentation *, IRepresentationStream *>     *audioRepresentations;
    uint32_t                                                            videoSegmentOffset;
    uint32_t                                                            audioSegmentOffset;
    size_t                                                              videoSegmentNumber;
    size_t                                                              audioSegmentNumber;
    bool                                                                videoSegmentIsSet;
    bool                                                                audioSegmentIsSet;
    int                                                                 videoSegmentQuality;
    int                                                                 audioSegmentQuality;
    bool                                                                isStopping;
};
}
}
}

#endif /* LIBDASH_FRAMEWORK_MPD_MPDWRAPPER_H_ */

