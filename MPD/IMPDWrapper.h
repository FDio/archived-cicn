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


#ifndef LIBDASH_FRAMEWORK_MPD_IMPDWRAPPER_H_
#define LIBDASH_FRAMEWORK_MPD_IMPDWRAPPER_H_

#include "../Managers/IStreamObserver.h"
//#include <string>
//#include <vector>
#include "IMPD.h"
#include "../Managers/IStreamObserver.h"

namespace libdash
{
namespace framework
{
namespace mpd
{
class IMPDWrapper
{
public:
    virtual std::string				getAvailabilityStarttime() = 0;
    virtual std::string				getTimeShiftBufferDepth() = 0;
    virtual std::string				getTypeWithoutLock() = 0;
    virtual uint32_t				getFetchTime() = 0;
    virtual std::string				getMinimumUpdatePeriodWithoutLock() = 0;
    virtual std::vector<dash::mpd::IBaseUrl *>	resolveBaseUrl(viper::managers::StreamType type, size_t mpdBaseUrl, size_t periodBaseUrl, size_t adaptationSetBaseUrl) = 0;
    virtual std::vector<dash::mpd::IBaseUrl *>	resolveBaseUrl(viper::managers::StreamType type, size_t mpdBaseUrl, size_t periodBaseUrl, size_t adaptationSetBaseUrl, dash::mpd::IMPD* mpd) = 0;
    virtual void				releaseLock() = 0;
    virtual std::string				getMediaPresentationDuration() = 0;
};
}
}
}

#endif /* LIBDASH_FRAMEWORK_MPD_IMPDWRAPPER_H_ */

