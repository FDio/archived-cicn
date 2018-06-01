/*
 * IStreamObserver.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef VIPER_MANAGERS_STREAMOBSERVER_H_
#define VIPER_MANAGERS_STREAMOBSERVER_H_

#include <stdint.h>

namespace viper
{
namespace managers
{
enum StreamType
{
    AUDIO = (1u << 0),
    VIDEO = (1u << 1),
    SUBTITLE = (1u << 2),
};

class IStreamObserver
{
public:
    virtual ~IStreamObserver () {}
    virtual void onSegmentDownloaded() = 0;
    virtual void onSegmentBufferStateChanged(StreamType type, uint32_t fillstateInPercent, int maxC) = 0;
    virtual void onVideoBufferStateChanged(uint32_t fillstateInPercent) = 0;
    virtual void onAudioBufferStateChanged(uint32_t fillstateInPercent) = 0;
    virtual void setEOS(bool value)	= 0;
    virtual void notifyStatistics(int segNum, uint32_t bitrate, int fps, uint32_t quality) = 0;
    virtual void notifyQualityDownloading(uint32_t quality) = 0;
    virtual bool canPush() = 0;
    virtual int  getBufferLevel() = 0;
    virtual void fetchMPD() = 0;
};
}
}

#endif /* VIPER_MANAGERS_STREAMOBSERVER_H_ */
