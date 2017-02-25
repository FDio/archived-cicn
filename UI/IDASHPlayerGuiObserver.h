/*
 * IDASHPlayerGuiObserver.h
 *****************************************************************************
 * Copyright (C) 2012, bitmovin Softwareentwicklung OG, All Rights Reserved
 *
 * Email: libdash-dev@vicky.bitmovin.net
 *
 * This source code and its use and distribution, is subject to the terms
 * and conditions of the applicable license agreement.
 *****************************************************************************/

#ifndef IDASHPLAYERGUIOBSERVER_H_
#define IDASHPLAYERGUIOBSERVER_H_

#include <string>
#include <qobject.h>
#include "ViperGui.h"

namespace viper
{
class IDASHPlayerGuiObserver : public QObject
{
    Q_OBJECT

public:
    virtual ~IDASHPlayerGuiObserver() {}
    virtual void onSettingsChanged(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation) = 0;
    virtual void onStartButtonPressed(int period, int videoAdaptationSet, int videoRepresentation, int audioAdaptationSet, int audioRepresentation, int adaptationLogic) = 0;
    virtual void onStopButtonPressed() = 0;
    virtual bool onDownloadMPDPressed(const std::string &url) = 0;
    virtual void onPauseButtonPressed() = 0;

};
}
#endif /* IDASHPLAYERGUIOBSERVER_H_ */
