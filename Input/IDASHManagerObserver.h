/*
 * IDASHManagerObserver.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_INPUT_IDASHMANAGEROBSERVER_H_
#define LIBDASH_FRAMEWORK_INPUT_IDASHMANAGEROBSERVER_H_

#include <QImage>
#include "../Buffer/Buffer.h"

namespace libdash
{
namespace framework
{
namespace input
{
class IDASHManagerObserver
{
public:
    virtual ~IDASHManagerObserver() {}

    virtual void setEOS(bool value)= 0;

    virtual void onSegmentBufferStateChanged(uint32_t fillstateInPercent, int maxC) = 0;
    virtual void notifyStatistics(int segNum, uint32_t bitrate, int fps, uint32_t quality) = 0;
    virtual void notifyQualityDownloading (uint32_t quality) = 0;
    virtual bool canPush() = 0;
    virtual int	 getBufferLevel() = 0;
};
}
}
}
#endif /* LIBDASH_FRAMEWORK_INPUT_IDASHMANAGEROBSERVER_H_ */
