/*
 * BaseUrlResolver.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_MPD_BASEURLRESOLVER_H_
#define LIBDASH_FRAMEWORK_MPD_BASEURLRESOLVER_H_

#include "IMPDWrapper.h"

namespace libdash
{
namespace framework
{
namespace mpd
{
class BaseUrlResolver
{
public:
    static std::vector<dash::mpd::IBaseUrl *> resolveBaseUrl(viper::managers::StreamType type, libdash::framework::mpd::IMPDWrapper *mpdWrapper,
                                                                 size_t mpdBaseUrl, size_t periodBaseUrl, size_t adaptationSetBaseUrl);
    static std::vector<dash::mpd::IBaseUrl *> resolveBaseUrl(viper::managers::StreamType type, libdash::framework::mpd::IMPDWrapper *mpdWrapper,
                                                                 size_t mpdBaseUrl, size_t periodBaseUrl, size_t adaptationSetBaseUrl, dash::mpd::IMPD* mpd);
};
}
}
}

#endif /* LIBDASH_FRAMEWORK_MPD_BASEURLRESOLVER_H_ */
