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

#include "GraphDataSource.h"
#include <QtCharts/QXYSeries>
#include <QtCharts/QAreaSeries>
#include <QtQuick/QQuickView>
#include <QtQuick/QQuickItem>
#include <QtCore/QDebug>
#include <QtCore/QtMath>
#include <QtCore/QDateTime>

QT_CHARTS_USE_NAMESPACE

Q_DECLARE_METATYPE(QAbstractSeries *)
Q_DECLARE_METATYPE(QAbstractAxis *)

GraphDataSource::GraphDataSource(QQuickView *appViewer, QObject *parent) :
    QObject(parent),
    m_appViewer(appViewer)
{
    qRegisterMetaType<QAbstractSeries*>();
    qRegisterMetaType<QAbstractAxis*>();
    index = 0;
    this->bitRate = -1;
    this->fps = -1;
    this->quality = -1;
    this->bufferLevel = -1;
}

void GraphDataSource::update(QAbstractSeries *bitRateSeries,  QAbstractSeries *qualitySeries)
{
    QXYSeries *xyBitRateSeries = static_cast<QXYSeries *>(bitRateSeries);
    QXYSeries *xyQualitySeries = static_cast<QXYSeries *>(qualitySeries);
    if (bitRatePoints.size() == 1000)
        bitRatePoints.remove(0);
    if (qualityPoints.size() == 1000)
        qualityPoints.remove(0);

    if(bitRate != -1)
    {
        bitRatePoints.append(QPointF(this->index, bitRate));
        xyBitRateSeries->replace(bitRatePoints);
    }

    if(quality != -1)
    {
        qualityPoints.append(QPointF(this->index, quality));
        xyQualitySeries->replace(qualityPoints);
    }
    this->index++;
}

void GraphDataSource::update(QAbstractSeries *bitRateSeries, QAbstractSeries *qualitySeries, QAbstractSeries *bufferLevelSeries)
{
    QXYSeries *xyBitRateSeries = static_cast<QXYSeries *>(bitRateSeries);
    QXYSeries *xyBufferLevelSeries = static_cast<QXYSeries *>(bufferLevelSeries);
    QXYSeries *xyQualitySeries = static_cast<QXYSeries *>(qualitySeries);
    if (bitRatePoints.size() == 1000)
        bitRatePoints.remove(0);
    if (bufferLevelPoints.size() == 1000)
        bufferLevelPoints.remove(0);
    if (qualityPoints.size() == 1000)
        qualityPoints.remove(0);
    if(bitRate != -1)
    {
        bitRatePoints.append(QPointF(this->index, bitRate));
        xyBitRateSeries->replace(bitRatePoints);
    }
    if(bufferLevel != -1)
    {
        bufferLevelPoints.append(QPointF(this->index, bufferLevel));
        xyBufferLevelSeries->replace(bufferLevelPoints);
    }
    if(quality != -1)
    {
        qualityPoints.append(QPointF(this->index, quality));
        xyQualitySeries->replace(qualityPoints);
    }

    this->index++;

}

void GraphDataSource::clearData()
{
    bitRatePoints.clear();
    fpsPoints.clear();
    qualityPoints.clear();
    bufferLevelPoints.clear();

    this->index = 0;
    this->bitRate = -1;
    this->fps = -1;
    this->quality = -1;
    this->bufferLevel = -1;
}

void GraphDataSource::setAnaliticsValues(uint32_t bitrate, int fps, uint32_t quality, double bufferLevel)
{
    this->bitRate = quality;
    this->fps = bufferLevel;
    this->quality = bitrate;
    this->bufferLevel = bufferLevel;
}

uint32_t GraphDataSource::getBitRate()
{
    return this->bitRate;
}

int GraphDataSource::getFps()
{
    return this->fps;
}

uint32_t GraphDataSource::getQuality()
{
    return this->bitRate;
}

double GraphDataSource::getBufferLevel()
{
    return this->bufferLevel;
}

void GraphDataSource::resetGraphValues()
{
    this->bitRate = -1;
    this->fps = -1;
    this->quality = -1;
    this->bufferLevel = -1;
}

