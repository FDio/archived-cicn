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
    signal closeOptionConnections
    signal saveAutotune(bool selectedAutotune)
    signal saveLifetime(int selectedLifetime)
    signal saveRetransmissions(int selectedRetransmissions)
    signal saveAlpha(real selectedAlpha)
    signal saveBeta(real selectedBeta)
    signal saveDrop(real selectedDrop)
    signal saveBetaWifi(real selectedBetaWifi)
    signal saveDropWifi(real selectedDropWifi)
    signal saveDelayWifi(int selectedDelayWifi)
    signal saveBetaLte(real selectedBetaLte)
    signal saveDropLte(real selectedDropLte)
    signal saveDelayLte(int selectedDelayLte)
    signal saveBatchingParameter(int selectedBatchingParameter)
    signal saveRateEstimator(int selectedRateEstimator)
    property int heightRow: Utils.scaled(60)

    id: root
    color: "#88445566"
    property variant target
    opacity: 0
    radius: Utils.scaled(10)
    anchors.top: parent.top
    anchors.left: parent.left
    anchors.right: parent.right
    anchors.bottom: parent.bottom
    enabled: false

    Item {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right

        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(12)
        id: itemAutotune
        Label {
            id: labelAdaptationSetList
            color: "white"
            anchors.top: parent.top
            anchors.right: comboAutotune.left
            anchors.rightMargin: Utils.scaled(5)

            anchors.topMargin: (comboAutotune.height - height)/2
            text: "Auto Tune"
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }

        ComboBox {
            z: parent.z + 1
            id: comboAutotune
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(200)
            width: Utils.scaled(200)
            enabled: true

            textRole: "text"
            model:  ListModel {
                id: adaptationLogicModel

                ListElement { text: "True"; }
                ListElement { text: "False"; }

            }
            onCurrentIndexChanged: {
                console.debug( currentIndex + " " + currentText)
            }
            currentIndex: autotune == true ? 0 : 1

        }
    }

    Item {
        id: itemLifetime
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(12) + heightRow
        Label {
            text: "Lifetime"
            id: labelLifetime
            color: " white"
            anchors.top: parent.top
            anchors.right: spinboxLifetime.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (spinboxLifetime.height - height)/2
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }
        SpinBox {
            id: spinboxLifetime
            z: parent.z + 1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(200)
            from: 1
            value: lifetime
            to: 10000
        }
    }

    Item {
        id: itemRetransmissions
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(12) + heightRow

        Label {
            text: "Retransmissions"
            id: labelRetransmissions
            color: " white"
            anchors.top: parent.top
            anchors.right: spinboxRetransmissions.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (spinboxRetransmissions.height - height)/2
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }
        SpinBox {
            id: spinboxRetransmissions
            z: parent.z + 1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(450)
            from: 1
            value: retransmissions
            to: 10000
            stepSize: 1
        }
    }

    Item {
        id: itemAlpha
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(12) + 2*heightRow

        Label {
            text: "Alpha"
            id: labelAlpha
            color: " white"
            anchors.top: parent.top
            anchors.right: spinboxAlpha.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (spinboxAlpha.height - height)/2
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }
        SpinBox {
            id: spinboxAlpha
            z: parent.z + 1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(200)
            from: 0
            value: alpha*100
            to: 10000
            stepSize: 1
            property int decimals: 2
            property real realValue: value / 100

            validator: DoubleValidator {
                bottom: Math.min(spinboxAlpha.from, spinboxAlpha.to)
                top:  Math.max(spinboxAlpha.from, spinboxAlpha.to)
            }

            textFromValue: function(value, locale) {
                return Number(value / 100).toLocaleString(locale, 'f', spinboxAlpha.decimals)
            }

            valueFromText: function(text, locale) {
                return Number.fromLocaleString(locale, text) * 100
            }
        }
    }

    Item {
        id: itemBeta
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(12) + heightRow * 2

        Label {
            text: "Beta"
            id: labelBeta
            color: " white"
            anchors.top: parent.top
            anchors.right: spinboxBeta.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (spinboxBeta.height - height)/2
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }
        SpinBox {
            id: spinboxBeta
            z: parent.z + 1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(450)
            from: 0
            value: beta*100
            to: 10000
            stepSize: 1

            property int decimals: 2
            property real realValue: value / 100

            validator: DoubleValidator {
                bottom: Math.min(spinboxBeta.from, spinboxBeta.to)
                top:  Math.max(spinboxBeta.from, spinboxBeta.to)
            }

            textFromValue: function(value, locale) {
                return Number(value / 100).toLocaleString(locale, 'f', spinboxBeta.decimals)
            }

            valueFromText: function(text, locale) {
                return Number.fromLocaleString(locale, text) * 100
            }
        }
    }

    Item {
        id: itemDrop
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(12) + heightRow * 2

        Label {
            text: "Drop"
            id: labelDrop
            color: " white"
            anchors.top: parent.top
            anchors.right: spinboxDrop.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (spinboxDrop.height - height)/2
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }

        SpinBox {
            id: spinboxDrop
            z: parent.z + 1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(700)
            from: 0
            value: drop*1000
            to: 100000
            stepSize: 1
            property int decimals: 3
            property real realValue: value / 1000

            validator: DoubleValidator {
                bottom: Math.min(spinboxDrop.from, spinboxDrop.to)
                top:  Math.max(spinboxDrop.from, spinboxDrop.to)
            }

            textFromValue: function(value, locale) {
                return Number(value / 1000).toLocaleString(locale, 'f', spinboxDrop.decimals)
            }

            valueFromText: function(text, locale) {
                return Number.fromLocaleString(locale, text) * 1000
            }
        }
    }


    Item {
        id: itemBetaWifi
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(12) + heightRow * 3

        Label {
            text: "BetaWifi"
            id: labelBetaWifi
            color: " white"
            anchors.top: parent.top
            anchors.right: spinboxBetaWifi.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (spinboxBetaWifi.height - height)/2
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }

        SpinBox {
            id: spinboxBetaWifi
            z: parent.z + 1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(200)
            from: 0
            value: betaWifi*100
            to: 10000
            stepSize: 1
            property int decimals: 2
            property real realValue: value / 100

            validator: DoubleValidator {
                bottom: Math.min(spinboxBetaWifi.from, spinboxBetaWifi.to)
                top:  Math.max(spinboxBetaWifi.from, spinboxBetaWifi.to)
            }

            textFromValue: function(value, locale) {
                return Number(value / 100).toLocaleString(locale, 'f', spinboxBetaWifi.decimals)
            }

            valueFromText: function(text, locale) {
                return Number.fromLocaleString(locale, text) * 100
            }
        }
    }

    Item {
        id: itemDropWifi
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(12) + heightRow * 3

        Label {
            text: "DropWifi"
            id: labelDropWifi
            color: " white"
            anchors.top: parent.top
            anchors.right: spinboxDropWifi.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (spinboxDropWifi.height - height)/2
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }

        SpinBox {
            id: spinboxDropWifi
            z: parent.z + 1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(450)
            from: 0
            value: dropWifi*1000
            to: 100000
            stepSize: 1
            property int decimals: 3
            property real realValue: value / 1000

            validator: DoubleValidator {
                bottom: Math.min(spinboxDropWifi.from, spinboxDropWifi.to)
                top:  Math.max(spinboxDropWifi.from, spinboxDropWifi.to)
            }

            textFromValue: function(value, locale) {
                return Number(value / 1000).toLocaleString(locale, 'f', spinboxDropWifi.decimals)
            }

            valueFromText: function(text, locale) {
                return Number.fromLocaleString(locale, text) * 1000
            }
        }
    }

    Item {
        id: itemDelayWifi
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(12) + heightRow * 3

        Label {
            text: "DelayWifi"
            id: labelDelayWifi
            color: " white"
            anchors.top: parent.top
            anchors.right: spinboxDelayWifi.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (spinboxDelayWifi.height - height)/2
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }

        SpinBox {
            id: spinboxDelayWifi
            z: parent.z + 1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(700)
            from: 1
            value: delayWifi
            to: 100000
            stepSize: 1
        }
    }

    Item {
        id: itemBetaLte
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(12) + heightRow * 4

        Label {
            text: "BetaLte"
            id: labelBetaLte
            color: " white"
            anchors.top: parent.top
            anchors.right: spinboxBetaLte.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (spinboxBetaLte.height - height)/2
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }

        SpinBox {
            id: spinboxBetaLte
            z: parent.z + 1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(200)
            from: 0
            value: betaLte*100
            to: 10000
            stepSize: 1
            property int decimals: 2
            property real realValue: value / 100

            validator: DoubleValidator {
                bottom: Math.min(spinboxBetaLte.from, spinboxBetaLte.to)
                top:  Math.max(spinboxBetaLte.from, spinboxBetaLte.to)
            }

            textFromValue: function(value, locale) {
                return Number(value / 100).toLocaleString(locale, 'f', spinboxBetaLte.decimals)
            }

            valueFromText: function(text, locale) {
                return Number.fromLocaleString(locale, text) * 100
            }
        }
    }

    Item {
        id: itemDropLte
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(12) + heightRow * 4

        Label {
            text: "DropLte"
            id: labelDropLte
            color: " white"
            anchors.top: parent.top
            anchors.right: spinboxDropLte.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (spinboxDropLte.height - height)/2
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }

        SpinBox {
            id: spinboxDropLte
            z: parent.z + 1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(450)
            from: 0
            value: dropLte*1000
            to: 100000
            stepSize: 1
            property int decimals: 3
            property real realValue: value / 1000

            validator: DoubleValidator {
                bottom: Math.min(spinboxDropLte.from, spinboxDropLte.to)
                top:  Math.max(spinboxDropLte.from, spinboxDropLte.to)
            }

            textFromValue: function(value, locale) {
                return Number(value / 1000).toLocaleString(locale, 'f', spinboxDropLte.decimals)
            }

            valueFromText: function(text, locale) {
                return Number.fromLocaleString(locale, text) * 1000
            }
        }
    }

    Item {
        id: itemDelayLte
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(12) + heightRow * 4

        Label {
            text: "DelayLte"
            id: labelDelayLte
            color: " white"
            anchors.top: parent.top
            anchors.right: spinboxDelayLte.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (spinboxDelayLte.height - height)/2
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }

        SpinBox {
            id: spinboxDelayLte
            z: parent.z + 1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(700)
            from: 1
            value: delayLte
            to: 100000
            stepSize: 1
        }
    }

    Item {
        id: itemBatchingParameter
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(12) + heightRow * 5

        Label {
            text: "Batching Parameter"
            id: labelBatchingParameter
            color: " white"
            anchors.top: parent.top
            anchors.right: spinboxBatchingParameter.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (spinboxBatchingParameter.height - height)/2
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }

        SpinBox {
            id: spinboxBatchingParameter
            z: parent.z + 1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(200)
            from: 1
            value: batchingParameter
            to: 100000
            stepSize: 1
        }
    }

    Item {
        id: itemRateEstimator
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(12) + heightRow * 5

        Label {
            text: "Rate Estimator"
            id: labelRateEstimator
            color: " white"
            anchors.top: parent.top
            anchors.right: spinboxRateEstimator.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (spinboxRateEstimator.height - height)/2
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }

        SpinBox {
            id: spinboxRateEstimator
            z: parent.z + 1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(450)
            from: 0
            value: rateEstimator
            to: 1
            stepSize: 1
        }
    }

    Item {
        id: itemButton
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: Utils.scaled(12)
        anchors.bottomMargin: Utils.scaled(12)

        Button {
            id: cancelBtn
            z: parent.z + 1
            text: "Cancel"
            anchors.right: saveBtn.left
            anchors.bottom: parent.bottom
            anchors.rightMargin: Utils.scaled(5)
            onClicked: {
                closeOptionConnections();
            }
        }

        Button {
            id: saveBtn
            z: parent.z + 1
            anchors.right: parent.right
            anchors.bottom: parent.bottom

            text: "Save"
            onClicked: {
                saveAutotune(comboAutotune.currentIndex == 0 ? true : false)
                saveLifetime(spinboxLifetime.value)
                saveRetransmissions(spinboxRetransmissions.value)
                saveAlpha(spinboxAlpha.value/100)
                saveBeta(spinboxBeta.value/100)
                saveDrop(spinboxDrop.value/1000)
                saveBetaWifi(spinboxBetaWifi.value/100)
                saveDropWifi(spinboxDropWifi.value/1000)
                saveDelayWifi(spinboxDelayWifi.value)
                saveBetaLte(spinboxBetaLte.value/100)
                saveDropLte(spinboxDropLte.value/1000)
                saveDelayLte(spinboxDelayLte.value)
                saveBatchingParameter(spinboxBatchingParameter.value)
                saveRateEstimator(spinboxRateEstimator.value)
                dashPlayer.reloadParameters()
                closeOptionConnections();
            }
        }
    }
}
