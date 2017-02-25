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

import QtQuick 2.0
import QtCharts 2.1


Rectangle {
    id: root
    anchors.fill: parent;
    color: Qt.rgba(0,0,0,0.5)
    ChartView {
        anchors.bottomMargin: parent.height/2
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.rightMargin: windowWidth*0.015

        id: chartViewBitRateFps
        opacity: 1
        animationOptions: ChartView.NoAnimation
        backgroundColor: "transparent"
        legend.visible: true
        legend.labelColor: "white"
        legend.font:Qt.font({pointSize: windowWidth*0.015, bold:true})
        antialiasing: enabled;
        property bool openGL: true
        onOpenGLChanged: {
            series("signal 1").useOpenGL = openGL;
        }

        ValueAxis {
            labelFormat: "%d%"
            id: bitBufferLevelY
            labelsColor: "white"

            labelsFont:Qt.font({pointSize: windowWidth*0.015, bold:true})
            min: 0
            max: 100

        }
        ValueAxis {
            labelFormat: "%d"
            id: bitRateAxisY
            labelsColor: "white"

            labelsFont:Qt.font({pointSize: windowWidth*0.015, bold:true})
            min: 0
            max: 20

        }

        ValueAxis {
            labelsVisible: false
            labelFormat: "%d"
            labelsAngle: 90
            labelsFont:Qt.font({pointSize: 1})
            id: axisX
            gridVisible: true
            min: -100
            max: 0
        }

        CategoryAxis {
            id: axesYQualityVideo
            min: 1
            max: 19
            gridVisible: false
            tickCount: 6
            labelsColor: "white"
            labelsFont:Qt.font({pointSize: windowWidth*0.015, bold:true})
            CategoryRange {
                label: "LD"
                endValue: 4
            }

            CategoryRange {
                label: "SD"
                endValue: 7
            }

            CategoryRange {
                label: "HD"
                endValue: 10
            }

            CategoryRange {
                label: "FHD"
                endValue: 13
            }

            CategoryRange {
                label: "QHD"
                endValue: 16
            }

            CategoryRange {
                label: "UHD"
                endValue: 19
            }
        }

        LineSeries {
            id: bufferLevelSeries
            name: "Buffer Level (%)"
            axisX: axisX
            color: "green"
            width: pixDens*3
            axisY: bitBufferLevelY
            useOpenGL: chartViewBitRateFps.openGL
        }

        LineSeries {
            id: bitRateSeries
            name: "Download Quality"
            axisX: axisX
            color: "yellow"
            width: pixDens*3
            axisYRight: axesYQualityVideo
            useOpenGL: chartViewBitRateFps.openGL
        }
    }

    ChartView {
        anchors.topMargin: parent.height/2
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        anchors.leftMargin: windowWidth*0.048
        id: chartViewQuality
        opacity: 1
        animationOptions: ChartView.NoAnimation
        backgroundColor: "transparent"
        legend.visible: true
        legend.labelColor: "white"
        legend.font:Qt.font({pointSize: windowWidth*0.015, bold:true})

        antialiasing: enabled;
        property bool openGL: true
        onOpenGLChanged: {
            series("signal 1").useOpenGL = openGL;
        }

        ValueAxis {
            labelsVisible: false
            labelFormat: "%d"
            labelsAngle: 90
            labelsFont:Qt.font({pointSize: 1})
            id: axisX2
            gridVisible: true
            min: -100
            max: 0
        }

        ValueAxis {
            labelFormat: "%d"
            id: bitRateAxisY2
            gridVisible: false
            labelsColor: "white"
            labelsFont:Qt.font({pointSize: windowWidth*0.015, bold:true})
            min: 0
            max: 20
        }

        LineSeries {
            id: dummySeries
            visible: false
            axisX: axisX2
            axisY: bitRateAxisY2
        }

        CategoryAxis {
            id: axeYQuality
            min: 1
            max: 19
            tickCount: 6
            labelsColor: "white"
            labelsFont:Qt.font({pointSize: windowWidth*0.015, bold:true})
            CategoryRange {
                label: "LD"
                endValue: 4
            }
            CategoryRange {
                label: "SD"
                endValue: 7
            }
            CategoryRange {
                label: "HD"
                endValue: 10
            }

            CategoryRange {
                label: "FHD"
                endValue: 13
            }

            CategoryRange {
                label: "QHD"
                endValue: 16
            }
            CategoryRange {
                label: "UHD"
                endValue: 19
            }
        }

        LineSeries {
            id: qualitySeries
            name: "Displayed Quality (Mbps)"
            axisX: axisX2
            width: pixDens*3
            color: "white"
            axisYRight: axeYQuality

            useOpenGL: chartViewQuality.openGL
        }
    }

    Timer {
        id: refreshTimer
        interval: 10
        objectName: "refreshTimer"
        running: false
        repeat: true
        onTriggered: {
            dataSource.update(bitRateSeries, qualitySeries, bufferLevelSeries);
            qualitySeries.axisX.min = qualitySeries.at(qualitySeries.count-1).x - 1000
            qualitySeries.axisX.max = qualitySeries.at(qualitySeries.count-1).x
            bitRateSeries.axisX.min = bitRateSeries.at(bitRateSeries.count-1).x - 1000
            bitRateSeries.axisX.max = bitRateSeries.at(bitRateSeries.count-1).x
        }
    }

    function startTimer()
    {
        refreshTimer.running = true;
    }

    function pauseTimer()
    {
        refreshTimer.running = false;
    }

    function stopTimer()
    {
        refreshTimer.running = false;
        bitRateSeries.clear();
        bufferLevelSeries.clear();
        qualitySeries.clear();
        dataSource.clearData();
    }
}

