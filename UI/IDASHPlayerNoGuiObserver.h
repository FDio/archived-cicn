/*
 * IDASHPlayerNoGuiObserver.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef IDASHPLAYERNOGUIOBSERVER_H_
#define IDASHPLAYERNOGUIOBSERVER_H_

#include <string>

namespace viper
{
class IDASHPlayerNoGuiObserver
{
public:
    virtual ~IDASHPlayerNoGuiObserver() {}
    virtual void onSettingsChanged(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation) = 0;
    virtual void onStartButtonPressed(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation) = 0;
    virtual void onStopButtonPressed() = 0;
    virtual bool onDownloadMPDPressed(const std::string &url) = 0;
};
}
#endif /* IDASHPLAYERNOGUIOBSERVER_H_ */
