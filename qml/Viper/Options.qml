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
    signal closeOptions
    signal saveAdaptationLogic(string selectedAdaptationLogic, int adaptationLogicKey)
    signal saveIcn(bool selectedIcn)
    signal saveVideoURI(string selectedVideoURI)
    signal saveSegmentBufferSize(real selectedSegmentBufferSize)
    signal saveRateAlpha(real selectedRateAlpha)
    signal saveBufferReservoirThreshold(real selectedBufferReservoirThreshold)
    signal saveBufferMaxThreshold(real selectedBufferMaxThreshold)
    signal saveAdaptechFirstThreshold(real selectedAdaptechFirstThreshold)
    signal saveAdaptechSecondThreshold(real selectedAdaptechSecondThreshold)
    signal saveAdaptechSwitchUpMargin(real selectedAdaptechSwitchUpMargin)
    signal saveAdaptechSlackParameter(real selectedAdaptechSlackParameter)
    signal saveAdaptechAlpha(real selectedAdaptechAlpha)
    signal saveBufferThreeThresholdFirst(real selectedBufferThreeThresholdFirst)
    signal saveBufferThreeThresholdSecond(real selectedBufferThreeThresholdSecond)
    signal saveBufferThreeThresholdThird(real selectedBufferThreeThresholdThird)
    signal savePandaParamAlpha(real selectedPandaParamAlpha)
    signal savePandaParamBeta(real selectedPandaParamBeta)
    signal savePandaParamBMin(real selectedPandaParamBMin)
    signal savePandaParamK(real selectedPandaParamK)
    signal savePandaParamW(real selectedPandaParamW)
    signal savePandaParamEpsilon(real selectedPandaParamEpsilon)
    signal saveBolaBufferTarget(real selectedBolaBufferTarget)
    signal saveBolaAlpha(real selectedBolaAlpha)
    signal reloadRateBasedConf
    signal reloadBufferBasedConf
    signal reloadBufferRateBasedConf
    signal reloadBufferThreeThresholdConf
    signal reloadPandaConf
    signal reloadBolaConf
    property int heightRow: Utils.scaled(60)

    function scaled(x)
    {
        return x*Screen.pixelDensity/Screen.logicalPixelDensity;
    }

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
        id: itemAdaptationSetList
        Label {
            id: labelAdaptationSetList
            color: "white"
            anchors.top: parent.top
            anchors.right: comboAdaptationSetList.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (comboAdaptationSetList.height - height)/2
            text: "Video AdaptationSet"
            font.bold: true
            font.pixelSize: Utils.scaled(10);

        }

        ComboBox {
            z: parent.z + 1
            id: comboAdaptationSetList
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(200)
            width: Utils.scaled(200)
            enabled: true
            textRole: "text"

            model:  ListModel {
                id: adaptationLogicModel
                ListElement { text: "Always Lowest"; }
                ListElement { text: "Rate Based"; }
                ListElement { text: "Buffer Based"; }
                ListElement { text: "AdapTech"; }
                ListElement { text: "Buffer Based Three Threshold"; }
                ListElement { text: "Panda"; }
                ListElement { text: "Bola"; }
            }

            onCurrentIndexChanged: {
                switch (currentIndex) {
                case 0:
                case 7:
                    reloadRateBasedConf()
                    spinboxRateAlpha.value = rateAlpha*100
                    rectangleRateBasedConf.enabled = false
                    rectangleRateBasedConf.opacity = 0
                    reloadBufferBasedConf()
                    spinboxBufferReservoirThreshold.value = bufferReservoirThreshold*100
                    spinboxBufferMaxThreshold.value = bufferMaxThreshold*100
                    rectangleBufferBasedConf.enabled = false
                    rectangleBufferBasedConf.opacity = 0
                    reloadBufferRateBasedConf()
                    spinboxAdaptechFirstThreshold.value = adaptechFirstThreshold*100
                    spinboxAdaptechSecondThreshold.value = adaptechSecondThreshold*100
                    spinboxAdaptechSwitchUpMargin.value = adaptechSwitchUpMargin*100
                    spinboxAdaptechSlackParameter.value = adaptechSlackParameter*100
                    spinboxAdaptechAlpha.value = adaptechAlpha*100
                    rectangleBufferRateBasedConf.enabled = false
                    rectangleBufferRateBasedConf.opacity = 0
                    reloadBufferThreeThresholdConf()
                    spinboxBufferThreeThresoldFirst.value = bufferThreeThresholdFirst*100
                    spinboxBufferThreeThresoldSecond.value = bufferThreeThresholdSecond*100
                    spinboxBufferThreeThresoldThird.value = bufferThreeThresholdThird*100
                    rectangleBufferThreeThresholdConf.enabled = false
                    rectangleBufferThreeThresholdConf.opacity = 0
                    reloadPandaConf()
                    spinboxPandaParamAlpha.value = pandaParamAlpha*100
                    spinboxPandaParamBeta.value = pandaParamBeta*100
                    spinboxPandaParamBMin.value = pandaParamBMin*100
                    spinboxPandaParamK.value = pandaParamK*100
                    spinboxPandaParamW.value = pandaParamW
                    spinboxPandaParamEpsilon.value = pandaParamEpsilon*100
                    rectanglePandaConf.enabled = false
                    rectanglePandaConf.opacity = 0
                    reloadBolaConf()
                    spinboxBolaBufferTarget.value = bolaBufferTarget*100
                    spinboxBolaAlpha.value = bolaAlpha*100
                    rectangleBolaConf.enabled = false
                    rectangleBolaConf.opacity = 0
                    break
                case 1:
                    rectangleRateBasedConf.enabled = true
                    rectangleRateBasedConf.opacity = 1
                    reloadBufferBasedConf()
                    spinboxBufferReservoirThreshold.value = bufferReservoirThreshold*100
                    spinboxBufferMaxThreshold.value = bufferMaxThreshold*100
                    rectangleBufferBasedConf.enabled = false
                    rectangleBufferBasedConf.opacity = 0
                    reloadBufferRateBasedConf()
                    spinboxAdaptechFirstThreshold.value = adaptechFirstThreshold*100
                    spinboxAdaptechSecondThreshold.value = adaptechSecondThreshold*100
                    spinboxAdaptechSwitchUpMargin.value = adaptechSwitchUpMargin*100
                    spinboxAdaptechSlackParameter.value = adaptechSlackParameter*100
                    spinboxAdaptechAlpha.value = adaptechAlpha*100
                    rectangleBufferRateBasedConf.enabled = false
                    rectangleBufferRateBasedConf.opacity = 0
                    reloadBufferThreeThresholdConf()
                    spinboxBufferThreeThresoldFirst.value = bufferThreeThresholdFirst*100
                    spinboxBufferThreeThresoldSecond.value = bufferThreeThresholdSecond*100
                    spinboxBufferThreeThresoldThird.value = bufferThreeThresholdThird*100
                    rectangleBufferThreeThresholdConf.enabled = false
                    rectangleBufferThreeThresholdConf.opacity = 0
                    reloadPandaConf()
                    spinboxPandaParamAlpha.value = pandaParamAlpha*100
                    spinboxPandaParamBeta.value = pandaParamBeta*100
                    spinboxPandaParamBMin.value = pandaParamBMin*100
                    spinboxPandaParamK.value = pandaParamK*100
                    spinboxPandaParamW.value = pandaParamW
                    spinboxPandaParamEpsilon.value = pandaParamEpsilon*100
                    rectanglePandaConf.enabled = false
                    rectanglePandaConf.opacity = 0
                    reloadBolaConf()
                    spinboxBolaBufferTarget.value = bolaBufferTarget*100
                    spinboxBolaAlpha.value = bolaAlpha*100
                    rectangleBolaConf.enabled = false
                    rectangleBolaConf.opacity = 0
                    break
                case 2:
                    reloadRateBasedConf()
                    spinboxRateAlpha.value = rateAlpha*100
                    rectangleRateBasedConf.enabled = false
                    rectangleRateBasedConf.opacity = 0
                    rectangleBufferBasedConf.enabled = true
                    rectangleBufferBasedConf.opacity = 1
                    reloadBufferRateBasedConf()
                    spinboxAdaptechFirstThreshold.value = adaptechFirstThreshold*100
                    spinboxAdaptechSecondThreshold.value = adaptechSecondThreshold*100
                    spinboxAdaptechSwitchUpMargin.value = adaptechSwitchUpMargin*100
                    spinboxAdaptechSlackParameter.value = adaptechSlackParameter*100
                    spinboxAdaptechAlpha.value = adaptechAlpha*100
                    rectangleBufferRateBasedConf.enabled = false
                    rectangleBufferRateBasedConf.opacity = 0
                    reloadBufferThreeThresholdConf()
                    spinboxBufferThreeThresoldFirst.value = bufferThreeThresholdFirst*100
                    spinboxBufferThreeThresoldSecond.value = bufferThreeThresholdSecond*100
                    spinboxBufferThreeThresoldThird.value = bufferThreeThresholdThird*100
                    rectangleBufferThreeThresholdConf.enabled = false
                    rectangleBufferThreeThresholdConf.opacity = 0
                    reloadPandaConf()
                    spinboxPandaParamAlpha.value = pandaParamAlpha*100
                    spinboxPandaParamBeta.value = pandaParamBeta*100
                    spinboxPandaParamBMin.value = pandaParamBMin*100
                    spinboxPandaParamK.value = pandaParamK*100
                    spinboxPandaParamW.value = pandaParamW
                    spinboxPandaParamEpsilon.value = pandaParamEpsilon*100
                    rectanglePandaConf.enabled = false
                    rectanglePandaConf.opacity = 0
                    reloadBolaConf()
                    spinboxBolaBufferTarget.value = bolaBufferTarget*100
                    spinboxBolaAlpha.value = bolaAlpha*100
                    rectangleBolaConf.enabled = false
                    rectangleBolaConf.opacity = 0
                    break
                case 3:
                    reloadRateBasedConf()
                    spinboxRateAlpha.value = rateAlpha*100
                    rectangleRateBasedConf.enabled = false
                    rectangleRateBasedConf.opacity = 0
                    reloadBufferBasedConf()
                    spinboxBufferReservoirThreshold.value = bufferReservoirThreshold*100
                    spinboxBufferMaxThreshold.value = bufferMaxThreshold*100
                    rectangleBufferBasedConf.enabled = false
                    rectangleBufferBasedConf.opacity = 0
                    rectangleBufferRateBasedConf.enabled = true
                    rectangleBufferRateBasedConf.opacity = 1
                    reloadBufferThreeThresholdConf()
                    spinboxBufferThreeThresoldFirst.value = bufferThreeThresholdFirst*100
                    spinboxBufferThreeThresoldSecond.value = bufferThreeThresholdSecond*100
                    spinboxBufferThreeThresoldThird.value = bufferThreeThresholdThird*100
                    rectangleBufferThreeThresholdConf.enabled = false
                    rectangleBufferThreeThresholdConf.opacity = 0
                    reloadPandaConf()
                    spinboxPandaParamAlpha.value = pandaParamAlpha*100
                    spinboxPandaParamBeta.value = pandaParamBeta*100
                    spinboxPandaParamBMin.value = pandaParamBMin*100
                    spinboxPandaParamK.value = pandaParamK*100
                    spinboxPandaParamW.value = pandaParamW
                    spinboxPandaParamEpsilon.value = pandaParamEpsilon*100
                    rectanglePandaConf.enabled = false
                    rectanglePandaConf.opacity = 0
                    reloadBolaConf()
                    spinboxBolaBufferTarget.value = bolaBufferTarget*100
                    spinboxBolaAlpha.value = bolaAlpha*100
                    rectangleBolaConf.enabled = false
                    rectangleBolaConf.opacity = 0
                    break
                case 4:
                    reloadRateBasedConf()
                    spinboxRateAlpha.value = rateAlpha*100
                    rectangleRateBasedConf.enabled = false
                    rectangleRateBasedConf.opacity = 0
                    reloadBufferBasedConf()
                    spinboxBufferReservoirThreshold.value = bufferReservoirThreshold*100
                    spinboxBufferMaxThreshold.value = bufferMaxThreshold*100
                    rectangleBufferBasedConf.enabled = false
                    rectangleBufferBasedConf.opacity = 0
                    reloadBufferRateBasedConf()
                    spinboxAdaptechFirstThreshold.value = adaptechFirstThreshold*100
                    spinboxAdaptechSecondThreshold.value = adaptechSecondThreshold*100
                    spinboxAdaptechSwitchUpMargin.value = adaptechSwitchUpMargin*100
                    spinboxAdaptechSlackParameter.value = adaptechSlackParameter*100
                    spinboxAdaptechAlpha.value = adaptechAlpha*100
                    rectangleBufferRateBasedConf.enabled = false
                    rectangleBufferRateBasedConf.opacity = 0
                    rectangleBufferThreeThresholdConf.enabled = true
                    rectangleBufferThreeThresholdConf.opacity = 1
                    reloadPandaConf()
                    spinboxPandaParamAlpha.value = pandaParamAlpha*100
                    spinboxPandaParamBeta.value = pandaParamBeta*100
                    spinboxPandaParamBMin.value = pandaParamBMin*100
                    spinboxPandaParamK.value = pandaParamK*100
                    spinboxPandaParamW.value = pandaParamW
                    spinboxPandaParamEpsilon.value = pandaParamEpsilon*100
                    rectanglePandaConf.enabled = false
                    rectanglePandaConf.opacity = 0
                    reloadBolaConf()
                    spinboxBolaBufferTarget.value = bolaBufferTarget*100
                    spinboxBolaAlpha.value = bolaAlpha*100
                    rectangleBolaConf.enabled = false
                    rectangleBolaConf.opacity = 0
                    break
                case 5:
                    reloadRateBasedConf()
                    spinboxRateAlpha.value = rateAlpha*100
                    rectangleRateBasedConf.enabled = false
                    rectangleRateBasedConf.opacity = 0
                    reloadBufferBasedConf()
                    spinboxBufferReservoirThreshold.value = bufferReservoirThreshold*100
                    spinboxBufferMaxThreshold.value = bufferMaxThreshold*100
                    rectangleBufferBasedConf.enabled = false
                    rectangleBufferBasedConf.opacity = 0
                    reloadBufferRateBasedConf()
                    spinboxAdaptechFirstThreshold.value = adaptechFirstThreshold*100
                    spinboxAdaptechSecondThreshold.value = adaptechSecondThreshold*100
                    spinboxAdaptechSwitchUpMargin.value = adaptechSwitchUpMargin*100
                    spinboxAdaptechSlackParameter.value = adaptechSlackParameter*100
                    spinboxAdaptechAlpha.value = adaptechAlpha*100
                    rectangleBufferRateBasedConf.enabled = false
                    rectangleBufferRateBasedConf.opacity = 0
                    reloadBufferThreeThresholdConf()
                    spinboxBufferThreeThresoldFirst.value = bufferThreeThresholdFirst*100
                    spinboxBufferThreeThresoldSecond.value = bufferThreeThresholdSecond*100
                    spinboxBufferThreeThresoldThird.value = bufferThreeThresholdThird*100
                    rectangleBufferThreeThresholdConf.enabled = false
                    rectangleBufferThreeThresholdConf.opacity = 0
                    rectanglePandaConf.enabled = true
                    rectanglePandaConf.opacity = 1
                    reloadBolaConf()
                    spinboxBolaBufferTarget.value = bolaBufferTarget*100
                    spinboxBolaAlpha.value = bolaAlpha*100
                    rectangleBolaConf.enabled = false
                    rectangleBolaConf.opacity = 0
                    break
                case 6:
                    reloadRateBasedConf()
                    spinboxRateAlpha.value = rateAlpha*100
                    rectangleRateBasedConf.enabled = false
                    rectangleRateBasedConf.opacity = 0
                    reloadBufferBasedConf()
                    spinboxBufferReservoirThreshold.value = bufferReservoirThreshold*100
                    spinboxBufferMaxThreshold.value = bufferMaxThreshold*100
                    rectangleBufferBasedConf.enabled = false
                    rectangleBufferBasedConf.opacity = 0
                    reloadBufferRateBasedConf()
                    spinboxAdaptechFirstThreshold.value = adaptechFirstThreshold*100
                    spinboxAdaptechSecondThreshold.value = adaptechSecondThreshold*100
                    spinboxAdaptechSwitchUpMargin.value = adaptechSwitchUpMargin*100
                    spinboxAdaptechSlackParameter.value = adaptechSlackParameter*100
                    spinboxAdaptechAlpha.value = adaptechAlpha*100
                    rectangleBufferRateBasedConf.enabled = false
                    rectangleBufferRateBasedConf.opacity = 0
                    reloadBufferThreeThresholdConf()
                    spinboxBufferThreeThresoldFirst.value = bufferThreeThresholdFirst*100
                    spinboxBufferThreeThresoldSecond.value = bufferThreeThresholdSecond*100
                    spinboxBufferThreeThresoldThird.value = bufferThreeThresholdThird*100
                    rectangleBufferThreeThresholdConf.enabled = false
                    rectangleBufferThreeThresholdConf.opacity = 0
                    reloadPandaConf()
                    spinboxPandaParamAlpha.value = pandaParamAlpha*100
                    spinboxPandaParamBeta.value = pandaParamBeta*100
                    spinboxPandaParamBMin.value = pandaParamBMin*100
                    spinboxPandaParamK.value = pandaParamK*100
                    spinboxPandaParamW.value = pandaParamW
                    spinboxPandaParamEpsilon.value = pandaParamEpsilon*100
                    rectanglePandaConf.enabled = false
                    rectanglePandaConf.opacity = 0
                    rectangleBolaConf.enabled = true
                    rectangleBolaConf.opacity = 1
                    break
                }
            }
            currentIndex: find(adaptationLogic)
        }

        Item {
            id: switchRectangle
            anchors.left: comboAdaptationSetList.right
            anchors.top: parent.top
            anchors.leftMargin: Utils.scaled(12)

            Label {
                text: "TCP"
                id: labelLegacy
                color: "white"
                anchors.top: parent.top
                anchors.left: parent.left
                font.bold: true
                font.pixelSize: switchIcn.height
            }

            Switch {
                id: switchIcn
                height: comboAdaptationSetList.height
                anchors.top: parent.top
                anchors.left: labelLegacy.right
                checked: icn
            }

            Label {
                id: labelIcn
                color: "white"
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.left: switchIcn.right
                text: "ICN"
                font.bold: true
                font.pixelSize: switchIcn.height
            }
        }
    }

     Item {
        id: itemVideoURI
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(18) + heightRow

        Label {
            text: "Video URI:"
            id: labelVideoURI
            color: " white"
            anchors.top: parent.top
            anchors.right: textInputVideoURI.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (textInputVideoURI.height - height)/2
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }

        TextInput  {
            width: parent.width/4*3
            id: textInputVideoURI
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(200)
            font.pixelSize: Utils.scaled(20)
            color: "white"
            text: videoURI
        }
    }

    Item {
        id: itemSegmentBufferSize
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.rightMargin: parent.width/2
        anchors.topMargin: Utils.scaled(12) + 3*heightRow

        Label {
            text: "Segment Buffer Size"
            id: labelSegmentBufferSize
            color: " white"
            anchors.top: parent.top
            anchors.right: spinboxSegmentBufferSize.left
            anchors.rightMargin: Utils.scaled(5)
            anchors.topMargin: (spinboxSegmentBufferSize.height - height)/2
            font.bold: true
            font.pixelSize: Utils.scaled(10);
        }

        SpinBox {
            id: spinboxSegmentBufferSize
            z: parent.z + 1
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.leftMargin: Utils.scaled(200)
            from: 0
            value: segmentBufferSize*100
            to: 10000
            stepSize: 100
            property int decimals: 0
            property real realValue: value / 100

            validator: DoubleValidator {
                bottom: Math.min(spinboxSegmentBufferSize.from, spinboxSegmentBufferSize.to)
                top:  Math.max(spinboxSegmentBufferSize.from, spinboxSegmentBufferSize.to)
            }

            textFromValue: function(value, locale)
            {
                return Number(value / 100).toLocaleString(locale, 'f', spinboxSegmentBufferSize.decimals)
            }

            valueFromText: function(text, locale)
            {
                return Number.fromLocaleString(locale, text) * 100
            }
        }
    }

    Rectangle {
        id: rectangleRateBasedConf
        z: parent.z + 1
        enabled: false
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: Utils.scaled(12) + 4*heightRow

        Label {
            text: "Rate Based Conf"
            id: labelRateBasedConf
            color: " white"
            anchors.top: parent.top
            anchors.leftMargin: Utils.scaled(10)
            anchors.left: parent.left
            font.bold: true
            font.pixelSize: Utils.scaled(20);
        }

        Item {
            id: itemRateAlpha
            anchors.top: labelRateBasedConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: parent.width/2
            anchors.topMargin: Utils.scaled(12)

            Label {
                text: "Rate\nAlpha"
                id: labelRateAlpha
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxRateAlpha.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxRateAlpha.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxRateAlpha
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(200)
                from: 0
                value: rateAlpha*100
                to: 100000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxRateAlpha.from, spinboxRateAlpha.to)
                    top:  Math.max(spinboxRateAlpha.from, spinboxRateAlpha.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxRateAlpha.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }
    }

    Rectangle {
        id: rectangleBufferBasedConf
        radius: Utils.scaled(10)
        z: parent.z + 1
        enabled: false
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: Utils.scaled(12) + 4*heightRow

        Label {
            text: "Buffer Based Conf"
            id: labelBufferBasedConf
            color: " white"
            anchors.top: parent.top
            anchors.leftMargin: Utils.scaled(10)
            anchors.left: parent.left
            font.bold: true
            font.pixelSize: Utils.scaled(20);
        }

        Item {
            id: itemBufferReservoirThreshold
            anchors.top: labelBufferBasedConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: parent.width/2
            anchors.topMargin: Utils.scaled(12)

            Label {
                text: "Buffer Reservoir\nThreshold"
                id: labelBufferReservoirThreshold
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxBufferReservoirThreshold.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxBufferReservoirThreshold.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxBufferReservoirThreshold
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(200)
                from: 0
                value: bufferReservoirThreshold*100
                to: 10000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxBufferReservoirThreshold.from, spinboxBufferReservoirThreshold.to)
                    top:  Math.max(spinboxBufferReservoirThreshold.from, spinboxBufferReservoirThreshold.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxBufferReservoirThreshold.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }

        Item {
            id: itemBufferMaxThreshold
            anchors.top: labelBufferBasedConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: parent.width/2
            anchors.topMargin: Utils.scaled(12)

            Label {
                text: "Buffer Max\nThreshold"
                id: labelBufferMaxThreshold
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxBufferMaxThreshold.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxBufferMaxThreshold.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxBufferMaxThreshold
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(450)
                from: 0
                value: bufferMaxThreshold*100
                to: 10000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxBufferMaxThreshold.from, spinboxBufferMaxThreshold.to)
                    top:  Math.max(spinboxBufferMaxThreshold.from, spinboxBufferMaxThreshold.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxBufferMaxThreshold.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }
    }

    Rectangle {
        id: rectangleBufferRateBasedConf
        z: parent.z + 1
        enabled: false
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: Utils.scaled(12) + 4*heightRow

        Label {
            text: "Buffer Rate Based Conf"
            id: labelBufferRateBasedConf
            color: " white"
            anchors.top: parent.top
            anchors.leftMargin: Utils.scaled(10)
            anchors.left: parent.left
            font.bold: true
            font.pixelSize: Utils.scaled(20);
        }

        Item {
            id: itemAdaptechFirstThreshold
            anchors.top: labelBufferRateBasedConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: parent.width/2
            anchors.topMargin: Utils.scaled(12)

            Label {
                text: "Adaptech First\nThreshold"
                id: labelAdaptechFirstThreshold
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxAdaptechFirstThreshold.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxAdaptechFirstThreshold.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxAdaptechFirstThreshold
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(200)
                from: 0
                value: adaptechFirstThreshold*100
                to: 10000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator{
                    bottom: Math.min(spinboxAdaptechFirstThreshold.from, spinboxAdaptechFirstThreshold.to)
                    top:  Math.max(spinboxAdaptechFirstThreshold.from, spinboxAdaptechFirstThreshold.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxAdaptechFirstThreshold.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }

        Item {
            id: itemAdaptechSecondThreshold
            anchors.top: labelBufferRateBasedConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: parent.width/2
            anchors.topMargin: Utils.scaled(12)

            Label {
                text: "Adaptech\nSecond\nThreshold"
                id: labelAdaptechSecondThreshold
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxAdaptechSecondThreshold.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxAdaptechSecondThreshold.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxAdaptechSecondThreshold
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(450)
                from: 0
                value: adaptechSecondThreshold*100
                to: 10000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxAdaptechSecondThreshold.from, spinboxAdaptechSecondThreshold.to)
                    top:  Math.max(spinboxAdaptechSecondThreshold.from, spinboxAdaptechSecondThreshold.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxAdaptechSecondThreshold.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }

        Item {
            id: itemAdaptechSwitchUpMargin
            anchors.top: labelBufferRateBasedConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: Utils.scaled(12)

            Label {
                text: "Adaptech\nSwitchUp\nMargin"
                id: labelAdaptechswitchUpMargin
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxAdaptechSwitchUpMargin.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxAdaptechSwitchUpMargin.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxAdaptechSwitchUpMargin
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(700)
                from: 0
                value: adaptechSwitchUpMargin*100
                to: 10000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxAdaptechSwitchUpMargin.from, spinboxAdaptechSwitchUpMargin.to)
                    top:  Math.max(spinboxAdaptechSwitchUpMargin.from, spinboxAdaptechSwitchUpMargin.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxAdaptechSwitchUpMargin.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }

        Item {
            id: itemAdaptechSlackParameter
            anchors.top: labelBufferRateBasedConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: Utils.scaled(12)

            Label {
                text: "Adaptech\nSlack\nParameter"
                id: labelAdaptechSwitchUpMargin
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxAdaptechSlackParameter.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxAdaptechSlackParameter.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxAdaptechSlackParameter
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(950)
                from: 0
                value: adaptechSlackParameter*100
                to: 10000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxAdaptechSlackParameter.from, spinboxAdaptechSlackParameter.to)
                    top:  Math.max(spinboxAdaptechSlackParameter.from, spinboxAdaptechSlackParameter.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxAdaptechSlackParameter.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }

        Item {
            id: itemAdaptechAlpha
            anchors.top: labelBufferRateBasedConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: parent.width/2
            anchors.topMargin: Utils.scaled(12) + heightRow

            Label {
                textFormat: Text.RichText
                text: "Adaptech\nAlpha"
                id: labelAdaptechAlpha
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxAdaptechAlpha.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxAdaptechAlpha.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxAdaptechAlpha
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(200)
                from: 0
                value: adaptechAlpha*100
                to: 10000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxAdaptechAlpha.from, spinboxAdaptechAlpha.to)
                    top:  Math.max(spinboxAdaptechAlpha.from, spinboxAdaptechAlpha.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxAdaptechAlpha.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }
    }

    Rectangle {
        id: rectangleBufferThreeThresholdConf
        radius: Utils.scaled(10)
        z: parent.z + 1
        enabled: false
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: Utils.scaled(12) + 4*heightRow

        Label {
            text: "Buffer Three Conf"
            id: labelBufferThreeThresholdConf
            color: " white"
            anchors.top: parent.top
            anchors.leftMargin: Utils.scaled(10)
            anchors.left: parent.left
            font.bold: true
            font.pixelSize: Utils.scaled(20);
        }

        Item {
            id: itemBufferThreeThresoldFirst
            anchors.top: labelBufferThreeThresholdConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: parent.width/2
            anchors.topMargin: Utils.scaled(12)

            Label {
                text: "Buffer Three\nThreshold\nFirst"
                id: labelBufferThreeThresoldFirst
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxBufferThreeThresoldFirst.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxBufferThreeThresoldFirst.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxBufferThreeThresoldFirst
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(200)
                from: 0
                value: bufferThreeThresholdFirst*100
                to: 10000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxBufferThreeThresoldFirst.from, spinboxBufferThreeThresoldFirst.to)
                    top:  Math.max(spinboxBufferThreeThresoldFirst.from, spinboxBufferThreeThresoldFirst.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxBufferThreeThresoldFirst.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }

        Item {
            id: itemBufferThreeThresoldSecond
            anchors.top: labelBufferThreeThresholdConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: parent.width/2
            anchors.topMargin: Utils.scaled(12)

            Label {
                text: "Buffer Three\nThreshold\nSecond"
                id: labelBufferThreeThresoldSecond
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxBufferThreeThresoldSecond.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxBufferThreeThresoldSecond.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxBufferThreeThresoldSecond
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(450)
                from: 0
                value: bufferThreeThresholdSecond*100
                to: 10000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxBufferThreeThresoldSecond.from, spinboxBufferThreeThresoldSecond.to)
                    top:  Math.max(spinboxBufferThreeThresoldSecond.from, spinboxBufferThreeThresoldSecond.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxBufferThreeThresoldSecond.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }

        Item {
            id: itemBufferThreeThresoldThird
            anchors.top: labelBufferThreeThresholdConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: Utils.scaled(12)

            Label {
                text: "Buffer Three\nThreshold\nThird"
                id: labelBufferThreeThresoldThird
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxBufferThreeThresoldThird.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxBufferThreeThresoldThird.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxBufferThreeThresoldThird
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(700)
                from: 0
                value: bufferThreeThresholdThird*100
                to: 10000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxBufferThreeThresoldThird.from, spinboxBufferThreeThresoldThird.to)
                    top:  Math.max(spinboxBufferThreeThresoldThird.from, spinboxBufferThreeThresoldThird.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxBufferThreeThresoldThird.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }
    }

    Rectangle {
        id: rectanglePandaConf
        radius: Utils.scaled(10)
        z: parent.z + 1
        enabled: false
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: Utils.scaled(12) + 4*heightRow

        Label {
            text: "Panda Conf"
            id: labelPandaConf
            color: " white"
            anchors.top: parent.top
            anchors.leftMargin: Utils.scaled(10)
            anchors.left: parent.left
            font.bold: true
            font.pixelSize: Utils.scaled(20);
        }

        Item {
            id: itemPandaParamAlpha
            anchors.top: labelPandaConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: parent.width/2
            anchors.topMargin: Utils.scaled(12)

            Label {
                textFormat: Text.RichText
                text: "Param &alpha;"
                id: labelPandaParamAlpha
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxPandaParamAlpha.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxPandaParamAlpha.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxPandaParamAlpha
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(200)
                from: 0
                value: pandaParamAlpha*100
                to: 10000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxPandaParamAlpha.from, spinboxPandaParamAlpha.to)
                    top:  Math.max(spinboxPandaParamAlpha.from, spinboxPandaParamAlpha.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxPandaParamAlpha.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }


        Item {
            id: itemPandaParamBeta
            anchors.top: labelPandaConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: parent.width/2
            anchors.topMargin: Utils.scaled(12)

            Label {
                textFormat: Text.RichText
                text: "Param &beta;"
                id: labelPandaParamBeta
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxPandaParamBeta.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxPandaParamBeta.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxPandaParamBeta
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(450)
                from: 0
                value: pandaParamBeta*100
                to: 10000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxPandaParamBeta.from, spinboxPandaParamBeta.to)
                    top:  Math.max(spinboxPandaParamBeta.from, spinboxPandaParamBeta.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxPandaParamBeta.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }

        Item {
            id: itemPandaParamBMin
            anchors.top: labelPandaConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: Utils.scaled(12)

            Label {
                textFormat: Text.RichText
                text: "Param B<sub>min</sub>"
                id: labelPandaParamBMin
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxPandaParamBMin.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxPandaParamBMin.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxPandaParamBMin
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(700)
                from: 0
                value: pandaParamBMin*100
                to: 10000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxPandaParamBMin.from, spinboxPandaParamBMin.to)
                    top:  Math.max(spinboxPandaParamBMin.from, spinboxPandaParamBMin.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxPandaParamBMin.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }

        Item {
            id: itemPandaParamK
            anchors.top: labelPandaConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.topMargin: Utils.scaled(12)

            Label {
                text: "Param K"
                id: labelPandaParamK
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxPandaParamK.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxPandaParamK.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxPandaParamK
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(950)
                from: 0
                value: pandaParamK*100
                to: 10000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxPandaParamK.from, spinboxPandaParamK.to)
                    top:  Math.max(spinboxPandaParamK.from, spinboxPandaParamK.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxPandaParamK.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }

        Item {
            id: itemPandaParamW
            anchors.top: labelPandaConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: parent.width/2
            anchors.topMargin: Utils.scaled(12) + heightRow

            Label {
                textFormat: Text.RichText
                text: "Param &omega;"
                id: labelPandaParamW
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxPandaParamW.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxPandaParamW.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxPandaParamW
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(200)
                from: 0
                value: pandaParamW
                to: 1000000
                stepSize: 1
            }
        }

        Item {
            id: itemPandaParamEpsilon
            anchors.top: labelPandaConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: parent.width/2
            anchors.topMargin: Utils.scaled(12) + heightRow

            Label {
                textFormat: Text.RichText
                text: "Param &epsilon;"
                id: labelPandaParamEpsilon
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxPandaParamEpsilon.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxPandaParamEpsilon.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxPandaParamEpsilon
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(450)
                from: 0
                value: pandaParamEpsilon*100
                to: 10000
                stepSize: 1
                property int decimals: 2
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxPandaParamEpsilon.from, spinboxPandaParamEpsilon.to)
                    top:  Math.max(spinboxPandaParamEpsilon.from, spinboxPandaParamEpsilon.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxPandaParamEpsilon.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }
    }

    Rectangle {
        id: rectangleBolaConf
        radius: Utils.scaled(10)
        z: parent.z + 1
        enabled: false
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.topMargin: Utils.scaled(12) + 4*heightRow

        Label {
            text: "Bola Conf"
            id: labelBolaConf
            color: " white"
            anchors.top: parent.top
            anchors.leftMargin: Utils.scaled(10)
            anchors.left: parent.left
            font.bold: true
            font.pixelSize: Utils.scaled(20);

        }

        Item {
            id: itemBolaBufferTarget
            anchors.top: labelBolaConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: Utils.scaled(parent.width/2)
            anchors.topMargin: Utils.scaled(12)

            Label {
                text: "Bola Buffer\nTarget"
                id: labelBolaBufferTarget
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxBolaBufferTarget.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxBolaBufferTarget.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxBolaBufferTarget
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(200)
                from: 0
                value: bolaBufferTarget*100
                to: 100000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxBolaBufferTarget.from, spinboxBolaBufferTarget.to)
                    top:  Math.max(spinboxBolaBufferTarget.from, spinboxBolaBufferTarget.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxBolaBufferTarget.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
        }

        Item {
            id: itemBolaAlpha
            anchors.top: labelBolaConf.bottom
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.rightMargin: parent.width/2
            anchors.topMargin: Utils.scaled(12)

            Label {
                text: "Bola\nAlpha"
                id: labelBolaAlpha
                color: " white"
                anchors.top: parent.top
                anchors.right: spinboxBolaAlpha.left
                anchors.rightMargin: Utils.scaled(5)
                anchors.topMargin: (spinboxBolaAlpha.height - height)/2
                font.bold: true
                font.pixelSize: Utils.scaled(10);
            }

            SpinBox {
                id: spinboxBolaAlpha
                z: parent.z + 1
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.leftMargin: Utils.scaled(450)
                from: 0
                value: bolaAlpha*100
                to: 10000
                stepSize: 10
                property int decimals: 1
                property real realValue: value / 100

                validator: DoubleValidator {
                    bottom: Math.min(spinboxBolaAlpha.from, spinboxBolaAlpha.to)
                    top:  Math.max(spinboxBolaAlpha.from, spinboxBolaAlpha.to)
                }

                textFromValue: function(value, locale)
                {
                    return Number(value / 100).toLocaleString(locale, 'f', spinboxBolaAlpha.decimals)
                }

                valueFromText: function(text, locale)
                {
                    return Number.fromLocaleString(locale, text) * 100
                }
            }
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
                closeOptions();
            }
        }

        Button {
            id: saveBtn
            z: parent.z + 1
            anchors.right: parent.right
            anchors.bottom: parent.bottom
            text: "Save"

            onClicked: {
                saveAdaptationLogic(adaptationLogicModel.get(comboAdaptationSetList.currentIndex).text, comboAdaptationSetList.currentIndex);
                saveIcn(switchIcn.checked)
                saveVideoURI(textInputVideoURI.text)
                saveSegmentBufferSize(spinboxSegmentBufferSize.value/100)
                saveRateAlpha(spinboxRateAlpha.value/100)
                saveBufferReservoirThreshold(spinboxBufferReservoirThreshold.value/100)
                saveBufferMaxThreshold(spinboxBufferMaxThreshold.value/100)
                saveAdaptechFirstThreshold(spinboxAdaptechFirstThreshold.value/100)
                saveAdaptechSecondThreshold(spinboxAdaptechSecondThreshold.value/100)
                saveAdaptechSwitchUpMargin(spinboxAdaptechSwitchUpMargin.value/100)
                saveAdaptechSlackParameter(spinboxAdaptechSlackParameter.value/100)
                saveAdaptechAlpha(spinboxAdaptechAlpha.value/100)
                saveBufferThreeThresholdFirst(spinboxBufferThreeThresoldFirst.value/100)
                saveBufferThreeThresholdSecond(spinboxBufferThreeThresoldSecond.value/100)
                saveBufferThreeThresholdThird(spinboxBufferThreeThresoldThird.value/100)
                savePandaParamAlpha(spinboxPandaParamAlpha.value/100)
                savePandaParamBeta(spinboxPandaParamBeta.value/100)
                savePandaParamBMin(spinboxPandaParamBMin.value/100)
                savePandaParamK(spinboxPandaParamK.value/100)
                savePandaParamW(spinboxPandaParamW.value)
                savePandaParamEpsilon(spinboxPandaParamEpsilon.value/100)
                saveBolaBufferTarget(spinboxBolaBufferTarget.value/100)
                saveBolaAlpha(spinboxBolaAlpha.value/100)
                dashPlayer.reloadParameters()
                closeOptions();
            }
        }
    }
}
