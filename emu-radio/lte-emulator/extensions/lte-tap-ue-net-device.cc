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

#include "lte-tap-ue-net-device.h"

#include <ns3/llc-snap-header.h>
#include <ns3/simulator.h>
#include <ns3/callback.h>
#include <ns3/node.h>
#include <ns3/packet.h>
#include <ns3/lte-net-device.h>
#include <ns3/packet-burst.h>
#include <ns3/uinteger.h>
#include <ns3/trace-source-accessor.h>
#include <ns3/pointer.h>
#include <ns3/enum.h>
#include <ns3/lte-amc.h>
#include <ns3/ipv4-header.h>
#include <ns3/lte-radio-bearer-tag.h>
#include <ns3/ipv4-l3-protocol.h>
#include <ns3/log.h>

//NOTE: this is used to handle the arp request. actually this is an hack
#include <ns3/arp-l3-protocol.h>
#include <ns3/arp-header.h>

#include <ns3/lte-enb-net-device.h>

/**
 * \brief LteTapUeNetDevice class extends lteUeNetDevice class to support emulation of lte channel
 */

namespace ns3 {
NS_LOG_COMPONENT_DEFINE ("LteTapUeNetDevice");

NS_OBJECT_ENSURE_REGISTERED ( LteTapUeNetDevice);

////////////////////////////////
// LteTapUeNetDevice
////////////////////////////////

TypeId LteTapUeNetDevice::GetTypeId (void)
{
  static TypeId
    tid =
    TypeId ("ns3::LteTapUeNetDevice")

    .SetParent<LteUeNetDevice> ()
    .AddConstructor<LteTapUeNetDevice>()
  ;
  return tid;
}

LteTapUeNetDevice::LteTapUeNetDevice()
 : m_virtualAddress(Mac48Address::Allocate())
{

}


void
LteTapUeNetDevice::SetPromiscReceiveCallback (PromiscReceiveCallback cb)
{
  NS_LOG_FUNCTION (this);
//   NS_LOG_WARN ("Promisc mode not supported");

  /** NOTE: set the promisc callback to be compatable with tap bridge
   */
  m_promiscRx = cb;
}


void
LteTapUeNetDevice::Receive (Ptr<Packet> p)
{
  NS_LOG_FUNCTION (this << p);
  m_rxCallback (this, p, Ipv4L3Protocol::PROT_NUMBER, Address ());

  /** NOTE: added for comptabability with tapbridge
   */
  if (!m_promiscRx.IsNull ())
    {
      Mac48Address from = Mac48Address::ConvertFrom (GetAddress());
      Mac48Address to = m_vmMacAdress;

      enum NetDevice::PacketType type = NetDevice::PACKET_HOST;
      m_promiscRx (this, p, Ipv4L3Protocol::PROT_NUMBER, from, to, type);
    }
}

bool
LteTapUeNetDevice::Send (Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
  NS_LOG_FUNCTION (this << dest << protocolNumber);
  if(protocolNumber== ArpL3Protocol::PROT_NUMBER)
  {
    Ptr<Packet> p=packet->Copy();
    ArpHeader arp;
    uint32_t size = p->RemoveHeader (arp);
    if (size == 0)
      {
        NS_LOG_LOGIC ("lte ue received ARP: but Cannot remove ARP header");
        return true;
      }

    if (!arp.IsRequest ())
      {
        NS_LOG_LOGIC ("received arp packet is not arp request, don't react to act it");
        return true;
      }

    //check if ip source address of arp request equal to that of this lte device. If NOT, don't send reply
    Ipv4Address lteDeviceIpAddress = GetNode ()->GetObject<Ipv4> ()
                                               ->GetAddress (1, 0)
                                               .GetLocal ();
    if (arp.GetSourceIpv4Address () != lteDeviceIpAddress || arp.GetDestinationIpv4Address () != m_bsIpAddress)
      return true;

    Ipv4Address myIp = arp.GetDestinationIpv4Address ();
    Ipv4Address toIp = arp.GetSourceIpv4Address ();
    Address toMac = arp.GetSourceHardwareAddress ();

    //generate arp reply
    ArpHeader replyArp;
    replyArp.SetReply (m_virtualAddress, myIp, toMac, toIp);
    Ptr <Packet> replyPacket = Create<Packet> ();
    replyPacket->AddHeader (replyArp);
    enum NetDevice::PacketType type = NetDevice::PACKET_HOST;

    m_promiscRx (this, replyPacket, ArpL3Protocol::PROT_NUMBER, m_virtualAddress, m_vmMacAdress, type);

    return true;
  }

  return LteUeNetDevice::Send (packet, dest, protocolNumber);
}

}
