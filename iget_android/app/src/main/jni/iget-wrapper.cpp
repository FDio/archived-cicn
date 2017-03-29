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

#include "iget-wrapper.hpp"

#include <stdlib.h>
#include <boost/property_tree/info_parser.hpp>
#include <boost/thread.hpp>
#include <mutex>
#include <icnet/icnet_socket_consumer.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <android/log.h>

typedef boost::posix_time::ptime Time;
typedef boost::posix_time::time_duration TimeDuration;

Time t1(boost::posix_time::microsec_clock::local_time());

class CallbackContainer
{
    public:
        CallbackContainer():
            work(new boost::asio::io_service::work(m_ioService)),
            m_handler(std::async(std::launch::async, [this]() { m_ioService.run(); }))
        {
            m_seenManifestSegments = 0;
            m_seenDataSegments = 0;
            m_byteCounter = 0;
        }

        ~CallbackContainer()
        {
            work.reset();
        }

        void processPayload(icnet::ConsumerSocket &c, const uint8_t *buffer, size_t bufferSize)
        {
            icnet::Name m_name;
            c.getSocketOption(icnet::GeneralTransportOptions::NAME_PREFIX, m_name);
            std::string filename = destinationPathString + "/" + m_name.toString().substr(1 + m_name.toString().find_last_of("/"));

            m_ioService.dispatch([buffer, bufferSize, filename]() {
                Time t3(boost::posix_time::microsec_clock::local_time());
                std::ofstream file(filename.c_str(), std::ofstream::binary);
                file.write((char *) buffer, bufferSize);
                file.close();
                Time t2(boost::posix_time::microsec_clock::local_time());
                TimeDuration dt = t2 - t1;
                TimeDuration dt3 = t3 - t1;
                long msec = dt.total_milliseconds();
                long msec3 = dt3.total_milliseconds();
                __android_log_print(ANDROID_LOG_INFO, "ProcessPayLoad", "Elapsed Time: %.2f seconds -- %.2f [Mbps] -- %.2f", msec/1000.0, bufferSize*8/msec/1000.0, bufferSize*8/msec3/1000.0);
            });
        }

        bool verifyData(icnet::ConsumerSocket &c, const icnet::ContentObject &content_object)
        {
            if (content_object.getContentType() == icnet::PayloadType::DATA) {
                __android_log_print(ANDROID_LOG_INFO, "VerifyData", "VERIFY CONTENT");
              } else if (content_object.getContentType() == icnet::PayloadType::MANIFEST) {
                __android_log_print(ANDROID_LOG_INFO, "VerifyData", "VERIFY MANIFEST");
            }
            return true;
        }

        void processLeavingInterest(icnet::ConsumerSocket &c, const icnet::Interest &interest)
        {
            //std::cout << "LEAVES " << interest.getName().toUri() << std::endl;
        }

        void setDestinationPathString(std::string destinationPathString)
        {

            this->destinationPathString = destinationPathString;
                 __android_log_print(ANDROID_LOG_ERROR, "setDestinationPathString", "xxx destinationPathString %s", this->destinationPathString.c_str());
        }

    private:
        std::string destinationPathString;
        int m_seenManifestSegments;
        int m_seenDataSegments;
        int m_byteCounter;
        boost::asio::io_service m_ioService;
        std::shared_ptr<boost::asio::io_service::work> work;
        std::future<void> m_handler;
};


JNIEXPORT void JNICALL
Java_com_cisco_ccnx_service_CCNxService_startIGet(JNIEnv *env, jobject obj, jstring downloadPath, jstring destinationPath)
{
    jboolean isCopy;
    const char *downloadPathChar = env->GetStringUTFChars(downloadPath, &isCopy);
    jboolean isCopyDestinatioPathChar;
    const char *destinationPathChar = env->GetStringUTFChars(destinationPath, &isCopyDestinatioPathChar);
    std::string destinationPathString(destinationPathChar, strlen(destinationPathChar));
    std::string url(downloadPathChar, strlen(downloadPathChar));
    std::string locator = "";
    std::string path = "";
    std::string name = "";
    size_t found = 0;
    size_t pathBegin = 0;
    found = url.find("//");
    pathBegin = url.find('/', found+2);
    locator = url.substr(found+2, pathBegin - (found+2));
    path = url.substr(pathBegin, std::string::npos);
    name = "ccnx:/" + locator + "/get" + path;
    icnet::ConsumerSocket c(icnet::Name(name.c_str()), icnet::TransportProtocolAlgorithms::RAAQM);
    CallbackContainer stubs;
    stubs.setDestinationPathString(destinationPathString);
    c.setSocketOption(icnet::ConsumerCallbacksOptions::CONTENT_RETRIEVED,
                         (icnet::ConsumerContentCallback) std::bind(&CallbackContainer::processPayload,
                                                             &stubs,
                                                             std::placeholders::_1,
                                                             std::placeholders::_2,
                                                             std::placeholders::_3));
    c.setSocketOption(icnet::ConsumerCallbacksOptions::INTEREST_OUTPUT,
                         (icnet::ConsumerInterestCallback) std::bind(&CallbackContainer::processLeavingInterest,
                                                              &stubs,
                                                              std::placeholders::_1,
                                                              std::placeholders::_2));
    c.consume(icnet::Name());
    c.stop();
}

