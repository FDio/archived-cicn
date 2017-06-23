/*
 * IMultimediaManagerObserver.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef VIPER_MANAGERS_IMULTIMEDIAMANAGEROBSERVER_H_
#define VIPER_MANAGERS_IMULTIMEDIAMANAGEROBSERVER_H_

#include <stdint.h>

namespace viper
{
namespace managers
{
class IMultimediaManagerObserver
{
public:
    virtual ~IMultimediaManagerObserver () {}
    virtual void onVideoBufferStateChanged(uint32_t fillstateInPercent) = 0;
    virtual void onVideoSegmentBufferStateChanged(uint32_t fillstateInPercent) = 0;
    virtual void onAudioBufferStateChanged(uint32_t fillstateInPercent) = 0;
    virtual void onAudioSegmentBufferStateChanged(uint32_t fillstateInPercent) = 0;
    virtual void onEOS() = 0;
    virtual void notifyStatistics(int, uint32_t, int, uint32_t) = 0;
    virtual void notifyQualityDownloading(uint32_t) = 0;
    virtual void setMPDWrapper(libdash::framework::mpd::MPDWrapper*) = 0;
};
}
}
#endif /* VIPER_MANAGERS_IMULTIMEDIAMANAGEROBSERVER_H_ */
