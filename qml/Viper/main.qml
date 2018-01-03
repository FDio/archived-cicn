/******************************************************************************
    QtAV:  Multimedia framework based on Qt and FFmpeg
    Copyright (C) 2012-2016 Wang Bin <wbsecg1@gmail.com>
*   This file is part of QtAV (from 2013)
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.
    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.
    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
******************************************************************************/

import QtQuick 2.0
import QtQuick.Dialogs 1.2
import QtAV 1.7
import QtQuick.Window 2.1
import QtQml 2.2
import "utils.js" as Utils

Rectangle {
    id: root
    layer.enabled: false
    objectName: "root"
    width: Screen.width
    height: Screen.height
    color: "black"
    signal requestFullScreen
    signal requestNormalSize
    property bool play: false
    property bool pause: false
    property bool stop: true
    property bool buffering: false
    property string adaptationLogic: ""
    property string videoURI: ""
    property real alpha: 0
    property real segmentBufferSize: 0
    property bool icn: false
    property real rateAlpha:0
    property real bufferReservoirThreshold: 0
    property real bufferMaxThreshold: 0
    property real adaptechFirstThreshold: 0
    property real adaptechSecondThreshold: 0
    property real adaptechSwitchUpMargin: 0
    property real adaptechSlackParameter: 0
    property real adaptechAlpha: 0
    property real bufferThreeThresholdFirst: 0
    property real bufferThreeThresholdSecond: 0
    property real bufferThreeThresholdThird: 0
    property real pandaParamAlpha: 0
    property real pandaParamBeta: 0
    property real pandaParamBMin: 0
    property real pandaParamK: 0
    property real pandaParamW: 0
    property real pandaParamEpsilon: 0
    property real bolaBufferTarget: 0
    property real bolaAlpha: 0
    property int screenWidth: Screen.width
    property int screenHeight: Screen.height
    property int windowWidth: Window.width
    property int pixDens: Math.ceil(Screen.pixelDensity)
    property int itemWidth: 25 * pixDens
    property int itemHeight: 10 * pixDens
    property int scaledMargin: 2 * pixDens
    property int fontSize: 5 * pixDens
    property string platformName: Qt.platform.os
    property bool isPlaying: false
    property string graphStatus: "stop"
    property bool hasMpd: false
    property bool canBuffer: false
    property alias keyboardFocus: myKeyboard.focus
    property bool repeat: false
    property bool graph: false
    property bool enabledFullScreen: false
    property bool autotune: false
    property int lifetime: 0
    property int retransmissions: 0
    property real beta: 0
    property real drop: 0
    property real betaWifi: 0
    property real dropWifi: 0
    property int delayWifi: 0
    property real betaLte: 0
    property real dropLte: 0
    property int delayLte: 0
    property int batchingParameter: 0
    property int rateEstimator: 0

    function init(argv)
    {
        console.log("init>>>>>screen density logical: " + Screen.logicalPixelDensity + " pixel: " + Screen.pixelDensity);
    }

    function initGraph(initGraphValue)
    {
        graph = initGraphValue
    }

    function initRepeat(initRepeatValue)
    {
        repeat = initRepeatValue
    }

    function initFullScreen(initFullScreen)
    {
        enabledFullScreen = initFullScreen
    }

    function setPlay()
    {
        play = true
        pause = false
        stop = false
        buffering = false
    }

    function setAdaptationLogic(initAdaptationLogic)
    {
        adaptationLogic = initAdaptationLogic
    }

    function setIcn(initIcn)
    {
        icn = initIcn
    }

    function setPause()
    {
        play = true
        pause = true
        stop = false
        buffering = false
    }

    function startGraph()
    {
        graphPanel.startTimer()
    }

    function stopGraph()
    {
        graphPanel.stopTimer()
    }

    function pauseGraph()
    {
        graphPanel.pauseTimer()
    }


    function setStop()
    {
        control.setStopState()
    }

    function setBuffering()
    {
        canBuffer = true
    }

    function unSetBuffering()
    {
        canBuffer = false
    }

    VideoOutput2 {
        id: videoOut
        focus: false
        opengl: true
        fillMode: VideoOutput.PreserveAspectFit
        anchors.fill: parent
        source: player
        orientation: 0
        property real zoom: 1
    }

    AVPlayer {
        id: player
        objectName: "player"
        autoPlay: true
        videoCodecPriority: PlayerConfig.decoderPriorityNames
        onPositionChanged: {
        }

        videoCapture {

        }

        onSourceChanged: {
            videoOut.zoom = 1
            videoOut.regionOfInterest = Qt.rect(0, 0, 0, 0)
        }

        onPlaying: {
            control.setPlayingState()
        }
        onSeekFinished: {
        }

        onInternalAudioTracksChanged: {
        }

        onExternalAudioTracksChanged: {
        }

        onInternalSubtitleTracksChanged: {
        }

        onStopped: {
            console.log("stopped ")
            dashPlayer.onStopped(player.duration);
        }

        onPaused: {
            console.log("else segment  paused")
        }

        onError: {
            if (error !== MediaPlayer.NoError) {
                msg.error(errorString)
            }
        }

        onVolumeChanged: {
        }

        onStatusChanged: {
        }

        onBufferProgressChanged: {
        }
    }

    Subtitle {
        id: subtitle
        player: player
        enabled: PlayerConfig.subtitleEnabled
        autoLoad: PlayerConfig.subtitleAutoLoad
        engines: PlayerConfig.subtitleEngines
        delay: PlayerConfig.subtitleDelay
        fontFile: PlayerConfig.assFontFile
        fontFileForced: PlayerConfig.assFontFileForced
        fontsDir: PlayerConfig.assFontsDir

        onContentChanged: { //already enabled
        }

        onLoaded: {
            msg.info(qsTr("Subtitle") + ": " + path.substring(path.lastIndexOf("/") + 1))
            console.log(msg.text)
        }
        onSupportedSuffixesChanged: {

        }

        onEngineChanged: { // assume a engine canRender is only used as a renderer
            subtitleItem.visible = canRender
            subtitleLabel.visible = !canRender
        }
        onEnabledChanged: {
            subtitleItem.visible = enabled
            subtitleLabel.visible = enabled
        }
    }

    Text {
        id: msg
        objectName: "msg"
        horizontalAlignment: Text.AlignHCenter
        font.pixelSize: Utils.scaled(20)
        style: Text.Outline
        styleColor: "green"
        color: "white"
        anchors.top: root.top
        width: root.width
        height: root.height / 4
        onTextChanged: {
            msg_timer.stop()
            visible = true
            msg_timer.start()
        }
        Timer {
            id: msg_timer
            interval: 2000
            onTriggered: msg.visible = false
        }
        function error(txt)
        {
            styleColor = "red"
            text = txt
        }
        function info(txt)
        {
            styleColor = "green"
            text = txt
        }
    }

    Item {
        id: myKeyboard
        anchors.fill: parent
        focus: true
        Keys.onPressed: {
            switch (event.key) {
            case Qt.Key_Escape:
               break;
            case Qt.Key_M:
                break
            case Qt.Key_Right:
                break
            case Qt.Key_Left:
                break
            case Qt.Key_Up:
                break
            case Qt.Key_Down:
                break
            case Qt.Key_Space:
                break
            case Qt.Key_Plus:
                break;
            case Qt.Key_Minus:
                break;
            case Qt.Key_F:
                break
            case Qt.Key_R:
                break;
            case Qt.Key_T:
                break;
            case Qt.Key_C:
                break
            case Qt.Key_A:
                break
            case Qt.Key_O:
               break;
            case Qt.Key_N:
                break
            case Qt.Key_B:
                break;
            case Qt.Key_G:
                break;
            case Qt.Key_Q:
                if (event.modifiers & Qt.ControlModifier)
                    Qt.quit()
                break;
            }
        }
    }
    DropArea {
        anchors.fill: root
        onEntered: {
            if (!drag.hasUrls)
                return;
            console.log(drag.urls)
        }
    }

    Text {

        text: "Buffering..."
        id: bufferingText
        font.pointSize: 3*windowWidth*0.01;
        color: "white"
        font.family : "Helvetica"
        opacity: canBuffer ? 1 : 0;
        anchors.right: parent.horizontalCenter
        anchors.verticalCenter: parent.verticalCenter
    }

    GraphPanel {

        id: graphPanel
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.left: parent.right
        opacity: {
            if (graph)
                opacity = 0.9
            else
                opacity = 0
        }

        anchors.leftMargin: parent.width/2
        anchors.bottomMargin: control.height

    }

    ControlPanel {
        id: control
        objectName: "control"

        anchors {
            left: parent.left
            bottom: parent.bottom
            right: parent.right
            leftMargin: Utils.scaled(12)
            rightMargin: Utils.scaled(12)
        }
        Behavior on opacity {NumberAnimation {duration: 300} }

        onStartGraph: graphPanel.startTimer()
        onStopGraph:graphPanel.stopTimer()
        onPauseGraph:graphPanel.pauseTimer()

        onDownloadMPD: mpdList.downloadMpd()

        onOpenOptions: {
            icn = dashPlayer.getIcn()
            adaptationLogic = dashPlayer.getAdaptationLogic()
            videoURI = dashPlayer.getVideoURI()
            segmentBufferSize = dashPlayer.getSegmentBufferSize()
            rateAlpha = dashPlayer.getRateAlpha()
            bufferReservoirThreshold = dashPlayer.getBufferReservoirThreshold()
            bufferMaxThreshold = dashPlayer.getBufferMaxThreshold()
            adaptechFirstThreshold = dashPlayer.getAdaptechFirstThreshold()
            adaptechSecondThreshold = dashPlayer.getAdaptechSecondThreshold()
            adaptechSwitchUpMargin = dashPlayer.getAdaptechSwitchUpMargin()
            adaptechSlackParameter = dashPlayer.getAdaptechSlackParameter()
            adaptechAlpha = dashPlayer.getAdaptechAlpha()
            bufferThreeThresholdFirst = dashPlayer.getBufferThreeThresholdFirst()
            bufferThreeThresholdSecond = dashPlayer.getBufferThreeThresholdSecond()
            bufferThreeThresholdThird = dashPlayer.getBufferThreeThresholdThird()
            pandaParamAlpha = dashPlayer.getPandaParamAlpha()
            pandaParamBeta = dashPlayer.getPandaParamBeta()
            pandaParamBMin = dashPlayer.getPandaParamBMin()
            pandaParamK = dashPlayer.getPandaParamK()
            pandaParamW = dashPlayer.getPandaParamW()
            pandaParamEpsilon = dashPlayer.getPandaParamEpsilon()
            bolaBufferTarget = dashPlayer.getBolaBufferTarget()
            bolaAlpha = dashPlayer.getBolaAlpha()
            options.enabled = true
            options.opacity = 0.9
            enabled = false
        }

        onOpenOptionConnections: {
            autotune = dashPlayer.getAutotune()
            lifetime = dashPlayer.getLifetime()
            retransmissions = dashPlayer.getRetransmissions()
            alpha = dashPlayer.getAlpha()
            console.log("alpha " + alpha)
            beta = dashPlayer.getBeta()
            drop = dashPlayer.getDrop()
            betaWifi = dashPlayer.getBetaWifi()
            dropWifi = dashPlayer.getDropWifi()
            delayWifi = dashPlayer.getDelayWifi()
            betaLte = dashPlayer.getBetaLte()
            dropLte = dashPlayer.getDropLte()
            delayLte = dashPlayer.getDelayLte()
            batchingParameter = dashPlayer.getBatchingParameter()
            rateEstimator = dashPlayer.getRateEstimator()
            optionConnections.enabled = true
            optionConnections.opacity = 0.9
            enabled = false
        }

        onOpenGraph: {
            graph = true
            dashPlayer.setGraph(true)
            graphPanel.opacity = 0.9
        }

        onHideGraph: {
            graph = false
            dashPlayer.setGraph(false)
            graphPanel.opacity = 0
        }

        onRepeatVideo: {
            repeat = true
            dashPlayer.setRepeat(true)
        }

        onDonotRepeatVideo: {
            repeat = false
            dashPlayer.setRepeat(false)
        }

        onResizeWindowFullScreen: {
            var xxx = Utils.scaled(Screen.width)
            var yyy = Utils.scaled(Screen.height)
            root.width = Utils.scaled(Window.width)
            root.height = Utils.scaled(Window.height)
        }

        onResizeWindow: {
            root.width = Utils.scaled(1024)
            root.height = Utils.scaled(768)
        }

        onSaveFullScreen: {
            enabledFullScreen = true
            dashPlayer.setFullScreen(true)
        }

        onSaveExitFullScreen: {
            enabledFullScreen = false
            dashPlayer.setFullScreen(false)
        }

        onTogglePause: {

        }
        onOpenFile: {}
        //IF_QT53
        onOpenUrl: {}
        MouseArea {
            id: ma2
            anchors.fill: parent
            hoverEnabled: true
            propagateComposedEvents: true

            onEntered: {
                if(player.playbackState === MediaPlayer.PlayingState)
                {
                    control.opacity = 1.0;
                }
                else
                {
                    control.opacity = 1.0;
                }
            }

            onExited: {
                if(player.playbackState === MediaPlayer.PlayingState)
                {
                    control.opacity = 0.0;
                }
            }
        }
    }

    Options {
        id: options
        enabled: false
        objectName: "openOption"

        anchors {
            left: parent.left
            bottom: parent.bottom
            right: parent.right
            top: parent.top
            topMargin: Utils.scaled(12)
            bottomMargin: control.height + Utils.scaled(12)
            leftMargin: Utils.scaled(12)
            rightMargin: Utils.scaled(12)

        }

        onCloseOptions: {
            control.uncheckOptionsBtn()
            options.enabled = false
            control.enabled = true
            enabled = false;
            opacity = 0
        }

        onSaveAdaptationLogic: {
            dashPlayer.setAdaptationLogic(selectedAdaptationLogic)
            adaptationLogic = selectedAdaptationLogic
        }

        onSaveIcn: {
            dashPlayer.setIcn(selectedIcn)
            icn = selectedIcn
        }

        onSaveVideoURI: {
            dashPlayer.setVideoURI(selectedVideoURI)
            videoURI = selectedVideoURI
        }

        onSaveSegmentBufferSize: {
            dashPlayer.setSegmentBufferSize(selectedSegmentBufferSize)
            segmentBufferSize = selectedSegmentBufferSize
        }

        onSaveRateAlpha: {
            dashPlayer.setRateAlpha(selectedRateAlpha)
            rateAlpha = selectedRateAlpha
        }

        onSaveBufferReservoirThreshold: {
            dashPlayer.setBufferReservoirThreshold(selectedBufferReservoirThreshold)
            bufferReservoirThreshold = selectedBufferReservoirThreshold
        }

        onSaveBufferMaxThreshold: {
            dashPlayer.setBufferMaxThreshold(selectedBufferMaxThreshold)
            bufferMaxThreshold = selectedBufferMaxThreshold
        }

        onSaveAdaptechFirstThreshold: {
            dashPlayer.setAdaptechFirstThreshold(selectedAdaptechFirstThreshold)
            adaptechFirstThreshold = selectedAdaptechFirstThreshold
        }

        onSaveAdaptechSecondThreshold: {
            dashPlayer.setAdaptechSecondThreshold(selectedAdaptechSecondThreshold)
            adaptechSecondThreshold = selectedAdaptechSecondThreshold
        }

        onSaveAdaptechSwitchUpMargin: {
            dashPlayer.setAdaptechSwitchUpMargin(selectedAdaptechSwitchUpMargin)
            adaptechSwitchUpMargin = selectedAdaptechSwitchUpMargin
        }

        onSaveAdaptechSlackParameter: {
            dashPlayer.setAdaptechSlackParameter(selectedAdaptechSlackParameter)
            adaptechSlackParameter = selectedAdaptechSlackParameter
        }

        onSaveAdaptechAlpha: {
            dashPlayer.setAdaptechAlpha(selectedAdaptechAlpha)
            adaptechAlpha = selectedAdaptechAlpha
        }

        onSaveBufferThreeThresholdFirst: {
            dashPlayer.setBufferThreeThresholdFirst(selectedBufferThreeThresholdFirst)
            bufferThreeThresholdFirst = selectedBufferThreeThresholdFirst
        }

        onSaveBufferThreeThresholdSecond: {
            dashPlayer.setBufferThreeThresholdSecond(selectedBufferThreeThresholdSecond)
            bufferThreeThresholdSecond = selectedBufferThreeThresholdSecond
        }

        onSaveBufferThreeThresholdThird: {
            dashPlayer.setBufferThreeThresholdThird(selectedBufferThreeThresholdThird)
            bufferThreeThresholdThird = selectedBufferThreeThresholdThird
        }

        onSavePandaParamAlpha: {
            dashPlayer.setPandaParamAlpha(selectedPandaParamAlpha)
            pandaParamAlpha = selectedPandaParamAlpha
        }

        onSavePandaParamBeta: {
            dashPlayer.setPandaParamBeta(selectedPandaParamBeta)
            pandaParamBeta = selectedPandaParamBeta
        }

        onSavePandaParamBMin: {
            dashPlayer.setPandaParamBMin(selectedPandaParamBMin)
            pandaParamBMin = selectedPandaParamBMin
        }

        onSavePandaParamK: {
            dashPlayer.setPandaParamK(selectedPandaParamK)
            pandaParamK = selectedPandaParamK
        }

        onSavePandaParamW: {
            dashPlayer.setPandaParamW(selectedPandaParamW)
            pandaParamW = selectedPandaParamW
        }

        onSavePandaParamEpsilon: {
            dashPlayer.setPandaParamEpsilon(selectedPandaParamEpsilon)
            pandaParamEpsilon = selectedPandaParamEpsilon
        }

        onSaveBolaBufferTarget: {
            dashPlayer.setBolaBufferTarget(selectedBolaBufferTarget)
            bolaBufferTarget = selectedBolaBufferTarget
        }

        onSaveBolaAlpha: {
            dashPlayer.setBolaAlpha(selectedBolaAlpha)
            bolaAlpha = selectedBolaAlpha
        }

        onReloadBufferBasedConf: {
            bufferReservoirThreshold = dashPlayer.getBufferReservoirThreshold()
            bufferMaxThreshold = dashPlayer.getBufferMaxThreshold()
        }

        onReloadBufferRateBasedConf:{
            adaptechFirstThreshold = dashPlayer.getAdaptechFirstThreshold()
            adaptechSecondThreshold = dashPlayer.getAdaptechSecondThreshold()
            adaptechSwitchUpMargin = dashPlayer.getAdaptechSwitchUpMargin()
            adaptechSlackParameter = dashPlayer.getAdaptechSlackParameter()
            adaptechAlpha = dashPlayer.getAdaptechAlpha()
        }

        onReloadBufferThreeThresholdConf: {
            bufferThreeThresholdFirst = dashPlayer.getBufferThreeThresholdFirst()
            bufferThreeThresholdSecond = dashPlayer.getBufferThreeThresholdSecond()
            bufferThreeThresholdThird = dashPlayer.getBufferThreeThresholdThird()
        }

        onReloadPandaConf: {
            pandaParamAlpha = dashPlayer.getPandaParamAlpha()
            pandaParamBeta = dashPlayer.getPandaParamBeta()
            pandaParamBMin = dashPlayer.getPandaParamBMin()
            pandaParamK = dashPlayer.getPandaParamK()
            pandaParamW = dashPlayer.getPandaParamW()
            pandaParamEpsilon = dashPlayer.getPandaParamEpsilon()

        }

        onReloadBolaConf: {
            bolaBufferTarget = dashPlayer.getBolaBufferTarget()

        }
    }

    OptionConnections {
        id: optionConnections
        enabled: false
        objectName: "openOption"

        anchors {
            left: parent.left
            bottom: parent.bottom
            right: parent.right
            top: parent.top
            topMargin: Utils.scaled(12)
            bottomMargin: control.height + Utils.scaled(12)
            leftMargin: Utils.scaled(12)
            rightMargin: Utils.scaled(12)
        }

        onCloseOptionConnections: {
            control.uncheckOptionConnectionsBtn()
            optionConnections.enabled = false
            control.enabled = true
            enabled = false;
            opacity = 0
        }

        onSaveAutotune: {
            dashPlayer.setAutotune(selectedAutotune)
            autotune = selectedAutotune
        }

        onSaveLifetime: {
            dashPlayer.setLifetime(selectedLifetime)
            lifetime = selectedLifetime
        }

        onSaveRetransmissions: {
            dashPlayer.setRetransmissions(selectedRetransmissions)
            retransmissions = selectedRetransmissions
        }

        onSaveAlpha: {
            dashPlayer.setAlpha(selectedAlpha)
            alpha = selectedAlpha
        }

        onSaveBeta: {
            dashPlayer.setBeta(selectedBeta)
            beta = selectedBeta
        }

        onSaveDrop: {
            dashPlayer.setDrop(selectedDrop)
            drop = selectedDrop
        }

        onSaveBetaWifi: {
            dashPlayer.setBetaWifi(selectedBetaWifi)
            betaWifi = selectedBetaWifi
        }

        onSaveDropWifi: {
            dashPlayer.setDropWifi(selectedDropWifi)
            dropWifi = selectedDropWifi
        }

        onSaveDelayWifi: {
            dashPlayer.setDelayWifi(selectedDelayWifi)
            delayWifi = selectedDelayWifi
        }

        onSaveBetaLte: {
            dashPlayer.setBetaLte(selectedBetaLte)
            betaLte = selectedBetaLte
        }

        onSaveDropLte: {
            dashPlayer.setDropLte(selectedDropLte)
            dropLte = selectedDropLte
        }

        onSaveDelayLte: {
            dashPlayer.setDelayLte(selectedDelayLte)
            delayLte = selectedDelayLte
        }

        onSaveBatchingParameter: {
            dashPlayer.setBatchingParameter(selectedBatchingParameter)
            batchingParameter = selectedBatchingParameter
        }

        onSaveRateEstimator: {
            dashPlayer.setRateEstimator(selectedRateEstimator)
            rateAlpha = selectedRateEstimator
        }
    }

    //IF_QT53
    Dialog {
        id: urlDialog
        standardButtons: StandardButton.Open | StandardButton.Cancel
        title: qsTr("Open a URL")
        Rectangle {
            color: "black"
            anchors.top: parent.top
            height: Utils.kItemHeight
            width: parent.width
            TextInput {
                id: urlEdit
                color: "orange"
                font.pixelSize: Utils.kFontSize
                anchors.fill: parent
            }
        }
        onAccepted: player.source = urlEdit.displayText
    }

    //ENDIF_QT53
    FileDialog {
        id: fileDialog
        title: "Please choose a media file"
        selectMultiple: true
        folder: PlayerConfig.lastFile
        onAccepted: {
            var sub, av
            for (var i = 0; i < fileUrls.length; ++i)
            {
                var s = fileUrls[i].toString()
                if (s.endsWith(".srt")
                        || s.endsWith(".ass")
                        || s.endsWith(".ssa")
                        || s.endsWith(".sub")
                        || s.endsWith(".idx") //vob
                        || s.endsWith(".mpl2")
                        || s.endsWith(".smi")
                        || s.endsWith(".sami")
                        || s.endsWith(".sup")
                        || s.endsWith(".txt"))
                    sub = fileUrls[i]
                else
                    av = fileUrls[i]
            }
            if (sub)
            {
                subtitle.autoLoad = false
                subtitle.file = sub
            } else
            {
                subtitle.autoLoad = PlayerConfig.subtitleAutoLoad
                subtitle.file = ""
            }
            if (av)
            {
                player.source = av
                PlayerConfig.lastFile = av
            }
        }
    }
    Connections {
        target: Qt.application
        onStateChanged: { //since 5.1
            if (Qt.platform.os === "winrt" || Qt.platform.os === "winphone") //paused by system
                return
            // winrt is handled by system
            switch (Qt.application.state) {
            case Qt.ApplicationSuspended:
            case Qt.ApplicationHidden:
                player.pause()
                break
            default:
                break
            }
        }
    }


}
