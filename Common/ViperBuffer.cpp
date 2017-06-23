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
#include <stdio.h>
#include "ViperBuffer.h"
ViperBuffer::ViperBuffer(QObject* parent) :
    QIODevice(parent)
{
    readMax = 32768;
    readBuffer = (uint8_t*)malloc(sizeof(uint8_t)*readMax);
    pthread_mutex_init(&(this->monitorMutex), NULL);
    qByteArrayVector.reserve(2);
    qByteArrayVector.push_back(new QByteArray());
    qByteArrayVector.push_back(new QByteArray());
    indexReadBuffer = 0;
    indexWriteBuffer = 0;
}

ViperBuffer::~ViperBuffer()
{
    pthread_mutex_destroy(&(this->monitorMutex));
    free(readBuffer);
}

bool ViperBuffer::isSequential() const
{
    return true;
}

bool ViperBuffer::open(OpenMode mode)
{
    setOpenMode(mode);
    return true;
}

void ViperBuffer::close()
{
    qByteArrayVector.clear();
    setOpenMode(NotOpen);
}

void ViperBuffer::clear()
{
    qByteArrayVector.at(0)->clear();
    qByteArrayVector.at(1)->clear();
    indexReadBuffer = 0;
    indexWriteBuffer = 0;
}

qint64 ViperBuffer::readData(char* data, qint64 maxSize)
{
    pthread_mutex_lock(&(this->monitorMutex));
    if ((maxSize = qMin(maxSize, qint64(qByteArrayVector.at(indexReadBuffer)->size()))) <= 0)
    {
        pthread_mutex_unlock(&(this->monitorMutex));
        return qint64(0);
    }
    memcpy(data, qByteArrayVector.at(indexReadBuffer)->constData(), maxSize);
    qByteArrayVector.at(indexReadBuffer)->remove(0,maxSize);
    pthread_mutex_unlock(&(this->monitorMutex));
    return maxSize;

}

qint64 ViperBuffer::writeData(libdash::framework::input::MediaObject* media)
{
    pthread_mutex_lock(&(this->monitorMutex));

    int ret = 0;
    int total = 0;
    ret = media->ReadInitSegment(readBuffer,readMax);
    total += ret;
    this->writeData((const char *)readBuffer, ret);
    ret = media->Read(readBuffer,readMax);
    while(ret)
    {
        total += ret;
        this->writeData((const char *)readBuffer, ret);
	
        ret = media->Read(readBuffer,readMax);
    }
    pthread_mutex_unlock(&(this->monitorMutex));
    return total;
}

qint64 ViperBuffer::writeData(const char* data, qint64 maxSize)
{
    qByteArrayVector.at(indexWriteBuffer)->append(data, maxSize);
    return maxSize;
}


void ViperBuffer::writeToNextBuffer()
{

    pthread_mutex_lock(&(this->monitorMutex));
    indexWriteBuffer = (indexWriteBuffer + 1 ) % 2;
    pthread_mutex_unlock(&(this->monitorMutex));

}

void ViperBuffer::readFromNextBuffer()
{
    pthread_mutex_lock(&(this->monitorMutex));
    indexReadBuffer = (indexReadBuffer + 1 ) % 2;
    pthread_mutex_unlock(&(this->monitorMutex));

}

