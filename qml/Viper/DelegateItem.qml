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

Rectangle {
    id: root
    width: Math.max(Utils.kItemWidth, itemText.contentWidth+8)
    height: itemText.contentWidth
    property color selectedColor: "#66ddaadd"
    property alias text: itemText.text
    color: "#99000000"
    signal clicked
    Text {
        id: itemText
        color: "white"
        anchors.fill: parent
        anchors.margins: 4
        font.pixelSize: Utils.kFontSize
        anchors.centerIn: parent
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }
    MouseArea {
        anchors.fill: parent
        onClicked: {
            root.state = "selected"
            root.clicked()
        }
    }
    states: [
        State {
            name: "selected"
            PropertyChanges {
                target: delegateItem
                color: selectedColor
            }
        }
    ]
    transitions: [
        Transition {
            from: "*"; to: "*"
            ColorAnimation {
                properties: "color"
                easing.type: Easing.OutQuart
                duration: 500
            }
        }
    ]
}
