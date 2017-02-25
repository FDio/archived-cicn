#ifndef VIPERBUFFER_H
#define VIPERBUFFER_H

#include <QIODevice>
#include <qobject.h>
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

#include <iostream>
#include <vector>
#include "../Input/MediaObject.h"

class ViperBuffer : public QIODevice
{
    Q_OBJECT
public:
    ViperBuffer(QObject* parent = 0);
    ~ViperBuffer();
    bool open(OpenMode mode);
    void close();
    bool isSequential() const;
    qint64 readData(char* data, qint64 maxSize);
    qint64 writeData(libdash::framework::input::MediaObject* segment);
    QByteArray* buffer();
    void clear();
    void writeToNextBuffer();
    void readFromNextBuffer();

private:
    std::vector<QByteArray*> qByteArrayVector;
    unsigned int indexReadBuffer;
    unsigned int indexWriteBuffer;
    pthread_mutex_t monitorMutex;
    int readMax;
    uint8_t* readBuffer;
    qint64 writeData(const char* data, qint64 maxSize);
    Q_DISABLE_COPY(ViperBuffer)
};
#endif // VIPERBUFFER_H
