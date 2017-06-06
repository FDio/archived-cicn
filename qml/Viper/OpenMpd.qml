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

import QtQuick 2.5
import QtQuick.Extras 1.4
import QtQuick.Controls.Styles 1.4
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3
import "utils.js" as Utils
Rectangle {
    signal closeOpenMpd
    signal saveAndPlayMpd(string newOpenMpd)

    id: root
    color: "#88445566"

    opacity: 0
    radius: Utils.scaled(10)
    height: Utils.scaled(100)
    width: Utils.scaled(300)
    enabled: false;
    GridLayout {
        id : grid
        z: parent.z + 1
        anchors.fill: parent
        rows    : 2
        columns : 2
        anchors.leftMargin: Utils.scaled(12)

        anchors.rightMargin: Utils.scaled(12)
        anchors.topMargin: Utils.scaled(12)
        anchors.bottomMargin: Utils.scaled(12)
        property double colMulti : grid.width / grid.columns
        property double rowMulti : grid.height / grid.rows

        function prefWidth(item)
        {
            return colMulti * item.Layout.columnSpan
        }

        function prefHeight(item)
        {
            return rowMulti * item.Layout.rowSpan
        }

        TextInput  {

            id: sourceTextInput
            text: lastPlayed
            Layout.rowSpan : 1
                 Layout.columnSpan : 2
                 Layout.preferredWidth  : parent.colMulti * 2 + Utils.scaled(5) //grid.prefWidth(this)
                 Layout.preferredHeight : parent.rowMulti//grid.prefHeight(this)
            color: "white"
        }
       // ComboBox {
       //     z: parent.z + 1
       //     id: comboBoxList
       //     Layout.rowSpan : 1
       //     Layout.columnSpan : 2
       //     Layout.preferredWidth  : parent.colMulti * 2 + Utils.scaled(5) //grid.prefWidth(this)
       //     Layout.preferredHeight : parent.rowMulti//grid.prefHeight(this)
       //
       //     onCurrentIndexChanged: {
       //     }

       //     model: ListModel {
       //         id: mpdItems
       //         ListElement { text: "gastown"; }
       //         ListElement { text: "sintel"; }
       //     }
       //     currentIndex: find(lastPlayed)
        //}

        Button {
            id: cancelBtn
            z: parent.z + 1

            text: "Cancel"
            Layout.rowSpan   : 1
            Layout.columnSpan: 1
            Layout.preferredWidth  : grid.prefWidth(this)
            Layout.preferredHeight : grid.prefHeight(this)
            onClicked: {

                closeOpenMpd();
            }
        }

        Button {
            id: downloadBtn
            z: parent.z + 1
            Layout.rowSpan   : 1
            Layout.columnSpan: 1
            Layout.preferredWidth  : grid.prefWidth(this)
            Layout.preferredHeight : grid.prefHeight(this)
            text: "Download"
            onClicked: {
                saveAndPlayMpd(sourceTextInput.text)
                closeOpenMpd();
            }


        }



    }

}

