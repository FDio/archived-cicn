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

#ifndef GRAPHDATASOURCE_H
#define GRAPHDATASOURCE_H

#include <QtCore/QObject>
#include <QtCharts/QAbstractSeries>

QT_BEGIN_NAMESPACE
class QQuickView;
QT_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

class GraphDataSource : public QObject
{
    Q_OBJECT
public:
    explicit GraphDataSource(QQuickView *appViewer, QObject *parent = 0);
    void setAnaliticsValues(uint32_t bitRate, int fps, uint32_t quality, double bufferLevel);
    uint32_t getBitRate();
    int getFps();
    double getBufferLevel();
    uint32_t getQuality();


Q_SIGNALS:

public slots:
    void clearData();
    void update(QAbstractSeries *bitRateSeries,QAbstractSeries *qualitySeries,  QAbstractSeries *bufferLevelSeries);
    void update(QAbstractSeries *bitRateSeries, QAbstractSeries *qualitySeries);
    void resetGraphValues();

private:
    QQuickView *m_appViewer;
    QVector<QPointF> bitRatePoints;
    QVector<QPointF> fpsPoints;
    QVector<QPointF> qualityPoints;
    QVector<QPointF> bufferLevelPoints;
    long int index;
    long int bitRate;
    int fps;
    int quality;
    double bufferLevel;

};

#endif // GRAPHDATASOURCE_H
