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

#ifndef LTE_TAP_UE_NET_DEVICE_H
#define LTE_TAP_UE_NET_DEVICE_H

#include <ns3/lte-ue-net-device.h>

/**
 * \brief LteTapUeNetDevice class extends lteUeNetDevice class to support emulation of lte channel
 */

namespace ns3 {


/**
 * \defgroup lte LTE Models
 *
 */

/**
 * \ingroup lte
 *
 * LteTapUeNetDevice overides the receive logic and overides SetPromiscReceiveCallback of class lteUeNetDevice,
 * to make it compatable with tapbridge emulation
 */
class LteTapUeNetDevice : public LteUeNetDevice
{
public:
  static TypeId GetTypeId (void);
  virtual void Receive (Ptr <Packet> p);
  virtual void SetPromiscReceiveCallback (PromiscReceiveCallback cb);
  LteTapUeNetDevice (void);

  //overwrite the implementation in LteUeNetDevice in order to deal with arp request
  virtual bool Send (Ptr <Packet> packet, const Address &dest, uint16_t protocolNumber);

  //TODO optional? if related tap device resides on Container
  void setMacAdressOnVM (Mac48Address vmMacAddress)
  {
    m_vmMacAdress = vmMacAddress;
  }

  void setBsIpAddress (Ipv4Address bsIp)
  {
    m_bsIpAddress = bsIp;
  }

 protected:
  NetDevice::PromiscReceiveCallback m_promiscRx;
  Mac48Address m_vmMacAdress;
  //used for generating arp reply only
  Mac48Address m_virtualAddress;
  Ipv4Address m_bsIpAddress;
};

} // namespace ns3

#endif /* LTE_TAP_UE_NET_DEVICE_H */
