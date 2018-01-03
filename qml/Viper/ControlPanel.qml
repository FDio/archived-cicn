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
import "utils.js" as Utils
import QtQuick.Window 2.1
import QtAV 1.4

Rectangle {
    id: root
    function scaled(x) {
        console.log("Screen " + screenPixelDensity + "; r: " + Screen.pixelDensity/Screen.logicalPixelDensity + "; size: " + Screen.width + "x" + Screen.height);
        console.log("screen density logical: " + Screen.logicalPixelDensity + " pixel: " + Screen.pixelDensity + "; " + x + ">>>" +x*Screen.pixelDensity/Screen.logicalPixelDensity);
        return x*Screen.pixelDensity/Screen.logicalPixelDensity;
    }

    color: "black"
    opacity: 0.9
    radius: Utils.scaled(10)
    height: Utils.scaled(80)
    width: itemWidth-25*pixDens

    property string playState: "stop"
    property int duration: 0
    property real volume: 1
    property bool mute: false
    property bool hiding: false
    signal startGraph
    signal pauseGraph
    signal stopGraph
    signal resizeWindowFullScreen
    signal resizeWindow
    signal openMpd
    signal openOptions
    signal openOptionConnections
    signal openGraph
    signal hideGraph
    signal repeatVideo
    signal donotRepeatVideo
    signal saveFullScreen
    signal saveExitFullScreen
    signal togglePause
    signal showInfo
    signal showHelp
    signal openFile
    signal openUrl
    signal downloadMPD

    function setPlayingProgress(value)
    {
        playState = "play"
    }

    function setStopState()
    {
        isPlaying = "stop"
        playBtn.checked = false

    }

    function setPlayingState() {
        playBtn.checked = true
        playState = "play"

    }

    function setPauseState() {
        playBtn.checked = false
        playState = "pause"
    }

    function toggleFullScreen() {
        fullScreenBtn.checked = !fullScreenBtn.checked
    }

    gradient: Gradient {
        GradientStop { position: 0.0; color: "#88445566" }
        GradientStop { position: 0.618; color: "#cc1a2b3a" }
        GradientStop { position: 1.0; color: "#ee000000" }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onHoveredChanged: {
            if (containsMouse) {
                if (timer.running) //timer may ran a few seconds(<3) ago
                    timer.stop();
                root.aniShow()
            } else {

            }
        }

        onPressed: {
            if (timer.running) //timer may ran a few seconds(<3) ago
                timer.stop();
            root.aniShow()
        }
    }

    ProgressBar {
        id: progress
        objectName: "progress"

        anchors {
            top: parent.top
            topMargin: Utils.scaled(8)
            left: parent.left
            leftMargin: Utils.scaled(20)
            right: parent.right
            rightMargin: Utils.scaled(20)
        }
        height: Utils.scaled(8)
        onValueChangedByUi: {
            dashPlayer.seekVideo(value);
        }

        onEnter: {
            if (playState == "stop")
                return
        }

        onLeave: {
            if (playState == "stop")
                return
        }
        onHoverAt: {
            if (playState == "stop")
                return;
        }
    }

    Item {
        layer.enabled: true
        anchors {
            top: progress.bottom
            bottom: parent.bottom
            left: parent.left
            right: parent.right
            margins: Utils.scaled(8)
        }

        Text {
            id: now
            objectName: "now"
            text: "00:00:00"//Utils.msec2string(progress.value*duration)
            anchors {
                top: parent.top
                topMargin: Utils.scaled(2)
                left: parent.left
            }
            color: "white"
            font {
                pixelSize: Utils.scaled(12) //or point size?
            }
        }
        Text {
            id: life
            objectName: "life"
            text: "00:00:00"//Utils.msec2string(duration)
            anchors {
                top: parent.top
                topMargin: Utils.scaled(2)
                right: parent.right
            }
            color: "white"
            font {
                pixelSize: Utils.scaled(12)
            }
        }
        Button {
            id: playBtn
            enabled: true
            objectName: "playBtn"
            anchors.centerIn: parent
            checkable: true
            bgColor: "transparent"
            bgColorSelected: "transparent"
            width: Utils.scaled(50)
            height: Utils.scaled(50)
            icon: "qrc:///qml/images/play.svg"
            iconChecked: "qrc:///qml/images/pause.svg"

            onClicked: {
                if (checked === true) {
                    console.log(adaptationLogic)
                    dashPlayer.downloadMPD(adaptationLogic, icn)
                } else {
                    dashPlayer.pause();
                }
            }
        }

        Button {
            id: stopBtn
            enabled: true
            anchors.verticalCenter: playBtn.verticalCenter
            anchors.right: playBtn.left
            bgColor: "transparent"
            bgColorSelected: "transparent"
            width: Utils.scaled(35)
            height: Utils.scaled(35)
            icon: "qrc:///qml/images/stop.svg"
            onClicked: {
                playBtn.checked = false
                isPlaying = false
                canBuffer = false
                dashPlayer.onStopButtonPressed()
            }
        }

        Button {
            id: fullScreenBtn
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(60)
            anchors.verticalCenter: parent.verticalCenter
            checkable: true
            bgColor: "transparent"
            bgColorSelected: "transparent"
            width: Utils.scaled(30)
            height: Utils.scaled(30)
            icon: "qrc:///qml/images/fullscreen.png"
            iconChecked: "qrc:///qml/images/fullscreen-selected.png"
            visible: (Qt.platform.os != "android")
            checked: enabledFullScreen
            onCheckedChanged: {
                if (checked) {
                    fullScreen()
                    saveFullScreen()
                } else {
                    exitFullScreen()
                    saveExitFullScreen()
                }
            }
        }

        Row {
            anchors.right: parent.right
            anchors.rightMargin: Utils.scaled(70)
            anchors.verticalCenter: parent.verticalCenter
            spacing: Utils.scaled(20)

            Button {
                id: graphBtn
                bgColor: "transparent"
                bgColorSelected: "transparent"
                checkable: true
                width: Utils.scaled(30)
                height: Utils.scaled(30)
                icon: "qrc:///qml/images/graph.png"
                iconChecked: "qrc:///qml/images/graph-selected.png"
                visible: true
                checked: graph
                onCheckedChanged: {
                    if ( !graphBtn.checked) {
                        hideGraph()
                    } else {
                        openGraph()
                    }
                }
            }

            Button {
                id: optionsBtn
                bgColor: "transparent"
                bgColorSelected: "transparent"
                checkable: true
                width: Utils.scaled(30)
                height: Utils.scaled(30)
                icon: "qrc:///qml/images/options.png"
                iconChecked: "qrc:///qml/images/options-selected.png"
                visible: true
                onCheckedChanged: {
                    if (checked)
                        openOptions()
                }
            }

            Button {
                id: optionConnectionsBtn
                bgColor: "transparent"
                bgColorSelected: "transparent"
                checkable: true
                width: Utils.scaled(30)
                height: Utils.scaled(30)
                icon: "qrc:///qml/images/option-connections.png"
                iconChecked: "qrc:///qml/images/option-connections-selected.png"
                visible: true
                onCheckedChanged: {

                    if (checked)
                        openOptionConnections()
                }
            }

   /*         Button {
                id: openBtn
                bgColor: "transparent"
                bgColorSelected: "transparent"
                checkable: true
                width: Utils.scaled(30)
                height: Utils.scaled(30)
                icon: "qrc:///qml/images/open.png"
                iconChecked: "qrc:///qml/images/open-selected.png"
                onCheckedChanged: {
                    if(checked)
                        openMpd()
                }
            } */
            Button {
                id: repeatBtn
                bgColor: "transparent"
                bgColorSelected: "transparent"
                checkable: true
                width: Utils.scaled(30)
                height: Utils.scaled(30)
                icon: "qrc:///qml/images/repeat.png"
                iconChecked: "qrc:///qml/images/repeat-selected.png"
                checked: repeat
                onCheckedChanged: {
                    if ( !repeatBtn.checked) {
                        donotRepeatVideo()
                    } else {
                        repeatVideo()
                    }

                }

            }
        }
    }

    Timer {
        id: timer
        interval: 3000
        onTriggered: {
            root.aniHide()
        }
    }

    function hideIfTimedout()
    {
        timer.start()
    }

    PropertyAnimation {
        id: anim
        target: root
        properties: "opacity"
        function reverse()
        {
            duration = 1500
            to = 0
            from = root.opacity
        }

        function reset()
        {
            duration = 200
            from = root.opacity
            to = 0.9
        }
    }

    function aniShow()
    {
        hiding = false
        anim.stop()
        anim.reset()
        anim.start()
    }

    function aniHide()
    {
        hiding = true
        anim.stop()
        anim.reverse()
        anim.start()
    }

    function toggleVisible()
    {
        if (hiding)
            aniShow()
        else
            aniHide()
    }

    function enable()
    {
        playBtn.enabled = true
        stopBtn.enabled = true
    }

    function disable()
    {
        playBtn.enabled = false
        stopBtn.enabled = false
    }


    function fullScreen()
    {
        requestFullScreen()
        resizeWindowFullScreen()
    }

    function exitFullScreen()
    {
        requestNormalSize()
        resizeWindow()
    }

    function checkRepeatButton()
    {
        repeatBtn.checked = !repeatBtn.checked;
    }

    function uncheckOpenBtn()
    {
        openBtn.checked = false;
    }

    function enableOpenBtn()
    {
        openBtn.enabled = true;
    }

    function uncheckOptionsBtn()
    {

        optionsBtn.checked = false;
    }

    function uncheckOptionConnectionsBtn()
    {

        optionConnectionsBtn.checked = false;
    }

    function enableOptionsBtn()
    {
        optionsBtn.enabled = true;
    }
}
