/*
 * Copyright (c) 2017 Cisco and/or its affiliates.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef VIPER_MANAGERS_IMULTIMEDIAMANAGERBASE_H_
#define VIPER_MANAGERS_IMULTIMEDIAMANAGERBASE_H_

#include "IMPD.h"

namespace viper
{
namespace managers
{
class IMultimediaManagerBase
{
public:
    virtual bool setVideoQuality(dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet *adaptationSet, dash::mpd::IRepresentation *representation) = 0;
    virtual bool setAudioQuality(dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet *adaptationSet, dash::mpd::IRepresentation *representation) = 0;
    virtual bool isStarted() = 0;
    virtual bool isStopping() = 0;
    virtual void shouldAbort(bool isVideo) = 0;
    virtual void setTargetDownloadingTime(bool,double) = 0;

};
}
}

#endif //VIPER_MANAGERS_IMULTIMEDIAMANAGERBASE_H_
