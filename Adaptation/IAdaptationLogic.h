/*
 * IAdaptationLogic.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_ADAPTATION_IADAPTATIONLOGIC_H_
#define LIBDASH_FRAMEWORK_ADAPTATION_IADAPTATIONLOGIC_H_
#include "../Input/MediaObject.h"
#include "../Input/DASHReceiver.h"
#include "IRepresentation.h"
#include "../Managers/IMultimediaManagerBase.h"

namespace libdash
{
namespace framework
{
namespace input
{
class DASHReceiver;
}
namespace adaptation
{
//#define START __LINE__
//ADAPTATIONLOGIC Count is an hack to have the number of adaptation logic that we can use
#define FOREACH_ADAPTATIONLOGIC(ADAPTATIONLOGIC) \
    ADAPTATIONLOGIC(AlwaysLowest)  \
    ADAPTATIONLOGIC(RateBased)  \
    ADAPTATIONLOGIC(BufferBased)  \
    ADAPTATIONLOGIC(AdapTech)  \
    ADAPTATIONLOGIC(BufferBasedThreeThreshold)  \
    ADAPTATIONLOGIC(Panda) \
    ADAPTATIONLOGIC(Bola)  \
    ADAPTATIONLOGIC(Count)  \


#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum LogicType {
    FOREACH_ADAPTATIONLOGIC(GENERATE_ENUM)
};

static const char *LogicType_string[] = {
    FOREACH_ADAPTATIONLOGIC(GENERATE_STRING)
};

class IAdaptationLogic
{
public:
    virtual ~IAdaptationLogic() {}

    virtual uint32_t getPosition() = 0;
    virtual void setPosition(uint32_t segmentNumber) = 0;
    virtual dash::mpd::IRepresentation* getRepresentation() = 0;
//    virtual void setRepresentation(dash::mpd::IPeriod *period,
//                                                             dash::mpd::IAdaptationSet *adaptationSet,
//                                                             dash::mpd::IRepresentation *representation)= 0;
    virtual LogicType getType() = 0;
    virtual bool isUserDependent()= 0;
    virtual void bitrateUpdate(uint64_t bps, uint32_t segNum) = 0;
    virtual void dLTimeUpdate(double time) = 0;
    virtual void bufferUpdate(uint32_t bufferfillstate, int maxC) = 0;
    virtual bool isRateBased() = 0;
    virtual bool isBufferBased() = 0;
    virtual void setMultimediaManager(viper::managers::IMultimediaManagerBase *mmManager)= 0;
    virtual void onEOS(bool value) = 0;
    virtual void checkedByDASHReceiver() = 0;
//    virtual void updateMPD(dash::mpd::IMPD* mpd) = 0;
};

struct AdaptationParameters
{
    int segmentBufferSize;
    double segmentDuration;

    //RATE BASED
    double	Rate_Alpha;

    //BUFFER BASED
    int	BufferBased_reservoirThreshold;
    int	BufferBased_maxThreshold;

    //BOLA
    double	Bola_Alpha;
    double	Bola_bufferTargetSeconds;

    //ADAPTECH
    double	Adaptech_Alpha;
    int	Adaptech_FirstThreshold;
    int	Adaptech_SecondThreshold;
    int	Adaptech_SwitchUpThreshold;
    double 	Adaptech_SlackParameter;

    //BUFFER  THREE THRESHOLDS
    int	BufferThreeThreshold_FirstThreshold;
    int	BufferThreeThreshold_SecondThreshold;
    int	BufferThreeThreshold_ThirdThreshold;
    double 	BufferThreeThreshold_slackParameter;
    //PANDA
    double	Panda_Alpha;
    double	Panda_Beta;
    double	Panda_Bmin;
    double	Panda_K;
    double	Panda_W;
    double	Panda_Epsilon;
};
}
}
}
#endif /* LIBDASH_FRAMEWORK_ADAPTATION_IADAPTATIONLOGIC_H_ */
