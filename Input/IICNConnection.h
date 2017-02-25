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

#ifndef QTPLAYER_INPUT_IICNCONNECTION_H_
#define QTPLAYER_INPUT_IICNCONNECTION_H_

#include "IConnection.h"
#include "IChunk.h"


namespace libdash {
namespace framework {

namespace input {

class IICNConnection : public dash::network::IConnection {

public:
    virtual ~IICNConnection(){};

    virtual int Read(uint8_t* data, size_t len) = 0;
    virtual void Init(dash::network::IChunk *chunk) = 0;
    virtual void InitForMPD(const std::string&) = 0;
    virtual void SetBeta(float beta) = 0;
    virtual void SetDrop(float drop) = 0;
};
}
}
}


#endif // QTPLAYER_INPUT_IICNCONNECTION_H_
