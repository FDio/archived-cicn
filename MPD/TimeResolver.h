/*
 * TimeResolver.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef LIBDASH_FRAMEWORK_MPD_TIMERESOLVER_H_
#define LIBDASH_FRAMEWORK_MPD_TIMERESOLVER_H_

#include <time.h>
#include <string>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
//#include "config.h"

namespace libdash
{
namespace framework
{
namespace mpd
{
class TimeResolver
{
public:
    static bool checkTimeInterval(std::string availabilityStartTime, std::string availabilityEndTime);
    static uint32_t getCurrentTimeInSec();
    static uint32_t getUTCDateTimeInSec(const std::string& datetime);
    static double getDurationInSec(const std::string& duration);

private:
    static struct tm* resolveUTCDateTime(const std::string& timeString);
    static struct tm* getCurrentUTCTime();
    static std::vector<int> splitToI(const std::string &s, char delim);
    static std::vector<std::string> splitToStr(const std::string &s, char delim);

};
}
}
}

#endif /* LIBDASH_FRAMEWORK_MPD_TIMERESOLVER_H_ */
