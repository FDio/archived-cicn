/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2011 Centre Tecnologic de Telecomunicacions de Catalunya (CTTC)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Xuan Zeng <xuan.zeng@irt-systemx.fr>
 */

#include <ns3/lte-helper.h>
#include <ns3/epc-helper.h>
#include <ns3/core-module.h>
#include <ns3/network-module.h>
#include <ns3/ipv4-global-routing-helper.h>
#include <ns3/internet-module.h>
#include <ns3/mobility-module.h>
#include <ns3/lte-module.h>
#include <ns3/csma-helper.h>
#include <ns3/tap-bridge-module.h>

#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <future>
#include <unordered_map>

#include "extensions/lte-tap-helper.h"
#include "extensions/lte-tap-ue-net-device.h"

#define UE_IP_CONFIGURABLE 1

#ifdef UE_IP_CONFIGURABLE
#include "extensions/tap-point-to-point-epc-helper.h"
#endif

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <net/if.h>

#define CONSTANT_POSITION "constant_position"
#define RANDOM_WAYPOINT   "random_waypoint"

#include "connection-pool.h"
#include "query.h"
#include "communication-protocol.h"
#include "src/lte-emulator.h"

#define DEFAULT_EXPERIMENT_ID "lte-emulation"
#define N_AP            1

using namespace ns3;

/**
 * this is a program to emulate a LTE channel with 1 EnodeB and multiple UE clients, which can be
 * connected to either containers or real machines. IP stack is used interally to forward packets through
 * epc core.
 */


/**
 * \brief an helper to assign an IP address to an existing network device
 *  NOTE: if the ip address passed as parameter has been assigned to another device before the call to this function,
 *  ns3 will crash and remind you about IP address conflicts.
 */
Ipv4InterfaceContainer AssignAnyIpv4Address (Ptr <NetDevice> device, Ipv4Address ipAddress)
{
  Ipv4AddressHelper ueAddrHelper;
  Ipv4Mask fakemask ("255.255.255.0");//the mask is irrelevant here.
  Ipv4Mask fakemask2 (ipAddress.Get () & fakemask.Get ());
  Ipv4Address id = Ipv4Address (ipAddress.Get () ^ fakemask2.Get ());

  ueAddrHelper.SetBase (ipAddress, fakemask2, id);
  return ueAddrHelper.Assign (NetDeviceContainer (device));

}

/**
 * \brief a callback to send arp reply when an arp request is received from outside the emulator,
 *  NOTE: we need to wirte code to handle arp request ourselves inside LTE emulator because ARP protocol is not
 *  intrinsically supported by a LTE device(on Enode B or UE). But arp should be needed by the container/VM outside the emulator
 *  program. We implement this callback to solve the incomptability between external container/VM and internal emulated LTE node
 *
 */
void
SendArpReply (std::vector <std::string> ueIpAddrs, Ptr <NetDevice> device, Ptr<const Packet> p, uint16_t protocol, const Address &from, const Address &to, NetDevice::PacketType packetType)
{
  Ptr <Packet> packet = p->Copy ();
  ArpHeader arp;
  uint32_t size = packet->RemoveHeader (arp);

  if (size == 0)
    return;

  if (!arp.IsRequest ())
    return;

  bool shouldREply = false;

  ///do nothing. Don't send arp reply) if destionation IP address is not equal to one UE's Ip address
  for (unsigned iUe = 0; iUe < ueIpAddrs.size (); iUe++)
    {
      if (arp.GetDestinationIpv4Address () == Ipv4Address (ueIpAddrs.at (iUe)
                                                                    .c_str ()))
        {
          shouldREply = true;
          break;
        }
    }

  if (!shouldREply)
    return;

  Ipv4Address myIp = arp.GetDestinationIpv4Address ();
  Ipv4Address toIp = arp.GetSourceIpv4Address ();
  Address toMac = arp.GetSourceHardwareAddress ();
  ArpHeader replyArp;

  replyArp.SetReply (device->GetAddress (), myIp, toMac, toIp);
  Ptr <Packet> replyPacket = Create<Packet> ();
  replyPacket->AddHeader (replyArp);
  device->Send (replyPacket, toMac, ArpL3Protocol::PROT_NUMBER);
}

/**
 * \brief overwrite mac address of emulated device such that it will be the same as that of tap device.
 *  This is require for communication with external containers/VM
 */
void setMac (Ptr <NetDevice> m_bridgedDevice, std::string m_tapDeviceName)
{
  int sock = socket (PF_UNIX, SOCK_DGRAM, 0);

  /// Bind to that socket and let the kernel allocate an endpoint
  struct sockaddr_un un;
  memset (&un, 0, sizeof (un));
  un.sun_family = AF_UNIX;
  bind (sock, (struct sockaddr *) &un, sizeof (sa_family_t));

  // Set the ns-3 device's mac address to the overlying container's mac address
  struct ifreq s;
  strncpy (s.ifr_name, m_tapDeviceName.c_str (), sizeof (s.ifr_name));

  int ioctlResult = ioctl (sock, SIOCGIFHWADDR, &s);

  if (ioctlResult == 0)
    {
      Mac48Address learnedMac;
      learnedMac.CopyFrom ((uint8_t *) s.ifr_hwaddr
                                        .sa_data);
      m_bridgedDevice->SetAddress (learnedMac);
    }

  close (sock);
}

/**
 * \brief populate arp cache on the emulated pgw node with mac of VM/container in advance,
 *  this is requred when external VM/container uses /32 ip addresses
 */
void
populateArpCache (Ptr <Node> pgw, NetDeviceContainer csmaDevices, Mac48Address bsMac, Ipv4Address bsIp, int numberOfNodes)
{
  Ptr <Ipv4L3Protocol> pgwIp = pgw->GetObject<Ipv4L3Protocol> ();
  NS_ASSERT (pgwIp != 0);
  ObjectVectorValue interfaces;
  pgwIp->GetAttribute ("InterfaceList", interfaces);
  Mac48Address PgwMacAddr = Mac48Address::ConvertFrom (csmaDevices.Get (1)
                                                                  ->GetAddress ());

  for (ObjectVectorValue::Iterator j = interfaces.Begin (); j != interfaces.End (); j++)
    {
      Ptr <Ipv4Interface> ipIface = (*j).second
                                        ->GetObject<Ipv4Interface> ();
      NS_ASSERT (ipIface != 0);
      //std::cout<<"ip addr="<<ipIface->GetAddress(0).GetLocal()<<"\n";
      Ptr <NetDevice> device = ipIface->GetDevice ();

      if (device == csmaDevices.Get (1))///the csma interface on pgw
        {
          Ptr <ArpCache> cache = ipIface->GetArpCache ();
          ArpCache::Entry *entry = cache->Add (bsIp);
          entry->MarkWaitReply (0);
          entry->MarkAlive (bsMac);
          entry->MarkPermanent ();
          break;
        }
    }
}

int main (int argc, char *argv[])
{

  //////////////////////
  //
  // parameter list:
  //
  /////////////////////
  double distance = 1; //in meters, initial distance between UEs and Enode B
  unsigned uplinkBW = 100;
  unsigned downlinkBW = 100;
  unsigned lteTxMode = 2;//0=SISO,1=Tx diversity,2=spatial multiplexing
  std::string pathLossModel = "ns3::Cost231PropagationLossModel";//ns3::FriisPropagationLossModel
  //FriisSpectrumPropagationLossModel//Cost231PropagationLossModel

  bool isUeFixed = false;///for debugging purpose only
  std::string fadingModel = "ns3::TraceFadingLossModel";
  std::string fadingTrace = "/usr/share/lte-emulator/fading_trace_EPA_3kmph.fad";
  bool isFading = true;
  int AmcModel = LteAmc::PiroEW2010;//PiroEW2010 or MiErrorModel
  unsigned rccTxBuffer = 1500 * 140;//140 packets
  bool isAMRccEnabled = false;
  bool isLogging = true;
  bool isIpPrint = false;

  GlobalValue::Bind ("SimulatorImplementationType", StringValue ("ns3::RealtimeSimulatorImpl"));
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));


  //////////////////////////
  //
  // Command line parsing
  //
  //////////////////////////

  std::string bs_x_str = "";
  std::string bs_y_str = "";
  std::string bs_name = "";
  std::string bs_tap = "";
  std::string sta_list_str = "";
  std::string sta_taps_str = "";

  std::string sta_ips_str = "";

  std::string sta_macs_str = "";
  std::string n_sta_str = "";
  std::string bs_mac_str = "";
  std::string experiment_id_str = "";
  std::string control_port_str = "";

  std::string bs_ip_str = "";

  CommandLine cmd;
  cmd.AddValue ("bs-tap", "Name of the tap between NS3 and the base station", bs_tap);
  cmd.AddValue ("n-sta", "Number of stations", n_sta_str);
  cmd.AddValue ("sta-list", "List of the stations of the simulation", sta_list_str);
  cmd.AddValue ("sta-taps", "List of the taps between NS3 and the mobile stations", sta_taps_str);

  cmd.AddValue ("sta-ips", "List of the IPs of the UEs in the format of 1.0.0.3/24,1.0.0.4/24 ...", sta_ips_str);

  cmd.AddValue ("sta-macs", "List of the macs of the mobile stations", sta_macs_str);
  cmd.AddValue ("bs-x", "X position of the Base Station", bs_x_str);
  cmd.AddValue ("bs-y", "Y position of the Base Station", bs_y_str);
  cmd.AddValue ("experiment-id", "Unique identifier for the experiment", experiment_id_str);
  cmd.AddValue ("bs-name", "Index of the base station", bs_name);
  cmd.AddValue ("bs-mac", "Base station MAC address", bs_mac_str);
  cmd.AddValue ("bs-ip", "Base station IP address, in the format of 192.0.0.3/24", bs_ip_str);

  cmd.AddValue ("control-port", "Control port for dynamically managing the stations movement", control_port_str);

  cmd.AddValue<double> ("distance", "Initial distance between the bs and the other stations", distance);

  ///parameters for configuring the lte channel
  cmd.AddValue ("txBuffer", "rcc tx buffer", rccTxBuffer);
  cmd.AddValue ("isFading", "whether to enable fading in the channel", isFading);
  cmd.AddValue ("fadingTrace", "the fading trace file name", fadingTrace);


  ///parameters for debugging the lte channel
  cmd.AddValue ("printIP", "whether to print IP addresses in simulation", isIpPrint);
  cmd.AddValue ("isUeFixed", "whether ue moves or not, this is used for testing without mobility server", isUeFixed);

  ///log physical rate used by LTE device
  cmd.AddValue ("logging", "whether to log statistics of lte mac and physical layer", isLogging);

  cmd.Parse (argc, argv);

  if (bs_tap == "" || n_sta_str == "" || sta_list_str == "" || sta_taps_str == "" || sta_macs_str == ""
      || sta_ips_str == "" ||

      bs_x_str == "" || bs_y_str == "" || bs_name == "" || bs_mac_str == "" ||

      bs_ip_str == "" || control_port_str == "" || experiment_id_str == "")
    {
      std::cerr << "Important parameters are missing!" << std::endl;
      return -1;
    }

  //////////////////////////////////
  //
  // further comandline parsing
  //
  //////////////////////////////////
  std::list <std::string> station_list;
  boost::split (station_list, sta_list_str, boost::is_any_of (","));

  std::list <std::string> taps_list;
  boost::split (taps_list, sta_taps_str, boost::is_any_of (","));

  std::list <std::string> macs_list;
  boost::split (macs_list, sta_macs_str, boost::is_any_of (","));

  std::list <std::string> sta_ips_list;
  boost::split (sta_ips_list, sta_ips_str, boost::is_any_of (","));


  //UE IPs
  std::vector <std::string> ueIpAddrs;
  std::string Ip1;
  std::string mask1;

  std::list<std::string>::const_iterator sta_ip_i;
  bool first = true;
  for (sta_ip_i = sta_ips_list.begin (); sta_ip_i != sta_ips_list.end (); sta_ip_i++)
    {
      std::list <std::string> ipComponents;
      boost::split (ipComponents, *sta_ip_i, boost::is_any_of ("/"));
      ueIpAddrs.push_back (ipComponents.front ());
      if (first)
        {
          Ip1 = ipComponents.front ();
          mask1 = "/" + (ipComponents.back ());
          first = false;
        }
    }

  ///BS IP configuration using the one passed from command line
  std::list <std::string> BsIpAddr;
  boost::split (BsIpAddr, bs_ip_str, boost::is_any_of ("/"));

  std::string bsIp = BsIpAddr.front ();
  std::string bsMask = "/" + (BsIpAddr.back ());

  ///number of UEs
  uint16_t numberOfNodes = (uint16_t) atoi (n_sta_str.c_str ());

  //maximum value of id portion of ip address(to avoid address conflicts)
  uint32_t maxIP = Ipv4Address (bsIp.c_str ()).Get ();//max first initialized to bs' ip
  for (uint16_t iUe = 0; iUe < numberOfNodes; iUe++)
    {
      uint32_t ueIP = Ipv4Address (ueIpAddrs.at (iUe)
                                            .c_str ()).Get ();
      if (ueIP > maxIP)
        maxIP = ueIP;
    }

  /**
   * TO handle the extrem case of all /32 ip address. while assigned /32 ip address might be replaced with /24 ones in the emulator, the external container/VM
   * can keep using /32 address, the behaviour shall be consistent even in case of /32 address are used
   */
  if (bsMask == "/32")
    bsMask = "/24";
  if (mask1 == "/32")
    mask1 = "/24";

  ///to check ip address duplicaton, since the ip address are assigned by lurch and not guaranteed to be in a sequential way
  uint32_t bsMaskInNumber = (Ipv4Mask (bsMask.c_str ()).Get ());
  uint32_t bsIpInNumber = Ipv4Address (bsIp.c_str ()).Get ();
  uint32_t bsPrefix = bsIpInNumber & bsMaskInNumber;
  uint32_t uePrefix = Ipv4Address (Ip1.c_str ()).Get () & (Ipv4Mask (mask1.c_str ()).Get ());
  uint32_t maxId = (maxIP & bsMaskInNumber) ^maxIP;
  uint32_t nextId = ((uint32_t) (2e32 - 1) ^ bsMaskInNumber) & (maxId + 1);
  uint32_t nextIP = bsPrefix | nextId;// nextIP that can be used, it's to be assigned to LTE gateway(pgw)
  uint32_t next2IP = 0;// the next next IP that can be used, it's to be assigned to LTE inner network
  uint32_t inc = 2;
  bool duplicated = true;

  int trials = 0;
  //after 10 trials, we will leave the ns3 system to detect duplicate IP addresses allocation and crash
  while (duplicated && trials
                       < 10)//normally we don't have address collisions, so we are not going to iterate many times to get a combination of IP address configuration matching requrements passed from command line
    {
      trials++;
      next2IP = (((int) (2e32 - 1) ^ bsMaskInNumber) & (maxId + inc)) | uePrefix;
      duplicated = false;
      if (next2IP == bsIpInNumber || next2IP + 1 == bsIpInNumber)
        duplicated = true;

      for (uint16_t iUe = 0; iUe < numberOfNodes; iUe++)
        {
          uint32_t ueIP = Ipv4Address (ueIpAddrs.at (iUe)
                                                .c_str ()).Get ();
          if (next2IP == ueIP || next2IP + 1 == ueIP)
            duplicated = true;
        }
      inc++;
    }





  //control port
  unsigned short control_port = (unsigned short) atoi (control_port_str.c_str ());

  //bs position
  double bs_x = atof (bs_x_str.c_str ());
  double bs_y = atof (bs_y_str.c_str ());


  //////////////////////////////////
  //
  //   configure lte channel
  //
  //////////////////////////////////

  Config::SetDefault ("ns3::LteEnbNetDevice::UlBandwidth", UintegerValue (uplinkBW));
  Config::SetDefault ("ns3::LteEnbNetDevice::DlBandwidth", UintegerValue (downlinkBW));
  Config::SetDefault ("ns3::LteEnbRrc::DefaultTransmissionMode", UintegerValue (lteTxMode));
  Config::SetDefault ("ns3::LteHelper::PathlossModel", StringValue (pathLossModel));
  Config::SetDefault ("ns3::LteHelper::Scheduler", StringValue ("ns3::PfFfMacScheduler"));

  ///NOTE: ref: https://sites.google.com/site/lteencyclopedia/lte-radio-link-budgeting-and-rf-planning, gives the following set of parameters. Not used in our emulator any more 
  /*
  Config::SetDefault("ns3::LteUePhy::TxPower", DoubleValue(24));
  Config::SetDefault("ns3::LteUePhy::NoiseFigure", DoubleValue(7.0));
  Config::SetDefault("ns3::LteEnbPhy::TxPower", DoubleValue(46));
  Config::SetDefault("ns3::LteEnbPhy::NoiseFigure", DoubleValue(2.0));
  */

  /*error model configuration*/
  if (AmcModel == LteAmc::PiroEW2010)
    {
      Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::PiroEW2010));
      Config::SetDefault ("ns3::LteAmc::Ber", DoubleValue (0.00005));
    }
  else
    {
      Config::SetDefault ("ns3::LteAmc::AmcModel", EnumValue (LteAmc::MiErrorModel));
    }

  if (isAMRccEnabled)
    Config::SetDefault ("ns3::LteEnbRrc::EpsBearerToRlcMapping", EnumValue (ns3::LteEnbRrc::RLC_AM_ALWAYS));

  ///rcc layer tx queue Buffer size in bytes, having impact on throughput performance, idealy should be equal to DBP
  Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (rccTxBuffer));//140 packets with 1500bytes

  ///create lte helper with our patch to support tap device and lte channel emulation
  Ptr <LteTapHelper> lteHelper = CreateObject<LteTapHelper> ();

  ///channel fading configuration
  if (isFading)
    {
      ///for fading configuration:
      lteHelper->SetAttribute ("FadingModel", StringValue (fadingModel));

      std::ifstream ifTraceFile;
      ifTraceFile.open (fadingTrace.c_str (), std::ifstream::in);
      if (ifTraceFile.good ()) //trace file can be found
        {
          lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue (fadingTrace));
        }
      else
        {
          ifTraceFile.close ();
          std::ifstream ifTraceFile2;
          ifTraceFile2.open ("fading-traces/fading_trace_EPA_3kmph.fad", std::ifstream::in);
          if (ifTraceFile2.good ())
            {
              lteHelper->SetFadingModelAttribute ("TraceFilename", StringValue ("fading-traces/fading_trace_EPA_3kmph.fad"));
            }
          else
            {
              std::cout << "WARNING: fading trace file not found, fading disabled\n";
            }
        }

      // these parameters have to set only in case of the trace format
      // differs from the standard one, that is
      // - 10 seconds length trace
      // - 10,000 samples
      // - 0.5 seconds for window size
      // - 100 RB
      lteHelper->SetFadingModelAttribute ("TraceLength", TimeValue (Seconds (10.0)));
      lteHelper->SetFadingModelAttribute ("SamplesNum", UintegerValue (10000));
      lteHelper->SetFadingModelAttribute ("WindowSize", TimeValue (Seconds (0.5)));
      lteHelper->SetFadingModelAttribute ("RbNum", UintegerValue (100));
    }

#ifdef UE_IP_CONFIGURABLE
  Ptr <TapPointToPointEpcHelper> epcHelper = CreateObject<TapPointToPointEpcHelper> (Ip1, mask1, maxIP
                                                                                                 + 2);//reserve one for pgw out interface
#else
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
#endif

  lteHelper->SetEpcHelper (epcHelper);

  Ptr <Node> pgw = epcHelper->GetPgwNode ();

  ///Create a ghost node for representing the container/VM on pgw side in the emulator
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr <Node> ghostNode = remoteHostContainer.Get (0);
  InternetStackHelper internet;
  internet.Install (remoteHostContainer);

  ///Create the Internet using CSMA link
  CsmaHelper csmah;
  NodeContainer csmaNodes (ghostNode);

  ///install internet stack for pgw node:
  csmaNodes.Add (pgw);
  csmah.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("1Gb/s")));
  csmah.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  csmah.SetChannelAttribute ("Delay", TimeValue (Seconds (0)));//small enough
  NetDeviceContainer csmaDevices = csmah.Install (csmaNodes);

  AssignAnyIpv4Address (csmaDevices.Get (0), bsIp.c_str ());//assign explicity an address to ghost/BS node because it must be the same as that in VM

  Ipv4Address addrghost = ghostNode->GetObject<Ipv4> ()
                                   ->GetAddress (1, 0)
                                   .GetLocal ();

  Ipv4InterfaceContainer interfacesipv4csma = AssignAnyIpv4Address (csmaDevices.Get (1), Ipv4Address (nextIP));;//next IP after max IP assigned to ue and enode B

  ///NOTE: this is to handle arp request
  pgw->RegisterProtocolHandler (MakeBoundCallback (&SendArpReply, ueIpAddrs), ArpL3Protocol::PROT_NUMBER, csmaDevices.Get (1)/*, true*/);
  ///set a route to ghost node only on pgw, otherwise packets for uplink cannot come back
  Ipv4StaticRoutingHelper ipv4RoutingHelper;

  ///uncoment the following to see ip routing table
  //  Ptr<OutputStreamWrapper> routintable = Create<OutputStreamWrapper>("routingtable",std::ios::out);
  //  ipv4RoutingHelper.PrintRoutingTableAt(Seconds(0), pgw, routintable);
  Ptr <Ipv4StaticRouting> pgwStaticRouting = ipv4RoutingHelper.GetStaticRouting (pgw->GetObject<Ipv4> ());
  pgwStaticRouting->AddHostRouteTo (Ipv4Address (bsIp.c_str ()), 2);

  ///add route for each UE manually to support /32 address for UE
  for (int ith = 0; ith < numberOfNodes; ith++)
    {
      pgwStaticRouting->AddHostRouteTo (Ipv4Address (ueIpAddrs.at (ith)
                                                              .c_str ()), 1);
    }

  if (isIpPrint)
    {
      std::cout << "ghost node IP address=" << addrghost << "\n";
      std::cout << "pgw IP addressOut=" << pgw->GetObject<Ipv4> ()
                                              ->GetAddress (2, 0)
                                              .GetLocal () << "\n";
      std::cout << "pgw IP addressIn=" << pgw->GetObject<Ipv4> ()
                                             ->GetAddress (1, 0)
                                             .GetLocal () << "\n";
    }

  ///ue and enodeB nodes
  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create (1);
  ueNodes.Create (numberOfNodes);

  ///populate arp cache, necessary if the external VM container are configured to use /32 IP addresses
  populateArpCache (pgw, csmaDevices, Mac48Address (bs_mac_str.c_str ()), Ipv4Address (bsIp.c_str ()), numberOfNodes);

  ////////////////////////////////////
  //
  // initial Mobility Configuration
  //
  ////////////////////////////////////

  /// Install Mobility Model for enodeB
  Ptr <ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (bs_x, bs_y, 0));
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (enbNodes);

  if (isUeFixed)
    {
      ///install mobility for UE
      Ptr <ListPositionAllocator> UEpositionAlloc = CreateObject<ListPositionAllocator> ();
      for (uint16_t i = 0; i < numberOfNodes; i++)
        {
          UEpositionAlloc->Add (Vector (distance * i, distance, 0));
        }

      mobility.SetPositionAllocator (UEpositionAlloc);
      mobility.Install (ueNodes);
    }
  else
    {
      // UEs mobility. By default the UEs start from the same position of the Enode B
      MobilityHelper staMobility;
      Ptr <ListPositionAllocator> UEpositionAlloc = CreateObject<ListPositionAllocator> ();
      for (uint16_t i = 0; i < numberOfNodes; i++)
        {
          UEpositionAlloc->Add (Vector (bs_x, bs_y + distance, 0.0));
        }
      staMobility.SetPositionAllocator (UEpositionAlloc);
      staMobility.SetMobilityModel ("ns3::WaypointMobilityModel", "InitialPositionIsWaypoint", BooleanValue (false));
      staMobility.Install (ueNodes);
    }



  /// Install LTE Devices to the enodeB and UE nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  /// Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
#ifdef UE_IP_CONFIGURABLE
  for (unsigned i = 0; i < ueLteDevs.GetN (); i++)
    ueIpIface.Add (epcHelper->AssignUeIpv4Address (ueLteDevs.Get (i), ueIpAddrs.at (i)
                                                                               .c_str ()));
#else
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
#endif

  /// Assign IP address to UEs, and set default gateways for them
  for (uint32_t u = 0; u < ueNodes.GetN (); ++u)
    {
      Ptr <Node> ueNode = ueNodes.Get (u);
      /// Set the default gateway for the UE
      Ptr <Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
      Ptr <Ipv4> ipv4ue = ueNode->GetObject<Ipv4> (); // Get Ipv4 instance of the node
      Ipv4Address addrUe = ipv4ue->GetAddress (1, 0)
                                 .GetLocal ();
      if (isIpPrint)
        {
          std::cout << "ue IP address=" << addrUe << "\n";
          std::cout << "ue gateway=" << epcHelper->GetUeDefaultGatewayAddress () << "\n";
        }
    }

  if (isIpPrint)
    {
      Ptr <Ipv4> ipv4enb = enbNodes.Get (0)
                                   ->GetObject<Ipv4> ();
      Ipv4Address addrenb = ipv4enb->GetAddress (1, 0)
                                   .GetLocal ();
      std::cout << "enb IP address to ue=" << addrenb << "\n";
    }



  /// Attach UEs to eNodeB
  for (uint16_t i = 0; i < numberOfNodes; i++)
    {
      lteHelper->Attach (ueLteDevs.Get (i), enbLteDevs.Get (0));
    }
  lteHelper->ActivateDedicatedEpsBearer (ueLteDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), EpcTft::Default ());

  //uncomment the following to enable full lte traces
  //lteHelper->EnableTraces ();
  // Uncomment to enable PCAP tracing
  //p2ph.EnablePcapAll("lena-epc-first");



  //////////////////////////////////////
  //
  // TapBridge devices configuration
  //
  /////////////////////////////////////

  TapBridgeHelper tapBridge;
  tapBridge.SetAttribute ("Mode", StringValue ("UseLocal"));

  std::unordered_map <std::string, ns3::Ptr<ns3::Node>> map_name_ns3node;
  std::list<std::string>::const_iterator station;
  std::list<std::string>::const_iterator tap_sta;
  std::list<std::string>::const_iterator mac_sta;
  uint32_t i;

  //install tapbridge for UEs
  for (tap_sta = taps_list.begin (), mac_sta = macs_list.begin (), station = station_list.begin (), i = 0;
       tap_sta != taps_list.end () && mac_sta != macs_list.end ()
       && station != station_list.end (); tap_sta++, mac_sta++, station++, i++)
    {
      map_name_ns3node[*station] = ueNodes.Get (i);
      ueLteDevs.Get (i)
               ->GetObject<LteTapUeNetDevice> ()
               ->setMacAdressOnVM (Mac48Address (mac_sta->c_str ()));
      ueLteDevs.Get (i)
               ->GetObject<LteTapUeNetDevice> ()
               ->setBsIpAddress (Ipv4Address (bsIp.c_str ()));

      tapBridge.SetAttribute ("DeviceName", StringValue (tap_sta->c_str ()));
      tapBridge.Install (ueNodes.Get (i), ueLteDevs.Get (i));
      setMac (ueLteDevs.Get (i), *tap_sta);
    }

  map_name_ns3node[bs_name] = enbNodes.Get (0);

  ///install tapbridge for eNodeB
  tapBridge.SetAttribute ("DeviceName", StringValue (bs_tap.c_str ()));
  tapBridge.Install (ghostNode, csmaDevices.Get (0));

  //NOTE: required to overwrite the mac address of emulated device so that it is the same as that of tap device
  setMac (csmaDevices.Get (0), bs_tap);

  /////////////////////////////////
  //
  // Start of the simulation
  //
  ////////////////////////////////

  if (isUeFixed)
    {
      Simulator::Stop ();
      Simulator::Run ();
      Simulator::Destroy ();
      return 0;
    }


  ////////////////////////////////////
  //
  // lte mac and physical layer loggin
  //
  ////////////////////////////////////

  if (isLogging)
    {
      lteHelper->NewEnableTxPhyTraces ();
    }

  auto handle = std::async (std::launch::async, [] ()
  {
    Simulator::Stop ();
    Simulator::Run ();
    Simulator::Destroy ();
  });



  //////////////////////////////////////////////////
  //
  // handle websocket control commands from outside
  //
  ///////////////////////////////////////////////////
  ns3::emulator::LteEmulator emulator (map_name_ns3node, lteHelper);

  /// Handler function for outcoming connections
  CommunicationProtocol protocol;

  HandlerFunction handler = [&emulator, &protocol] (Server *s, websocketpp::connection_hdl hdl, message_ptr msg, const uint8_t *data, std::size_t size)
  {
    std::string command ((char *) data, size);
    boost::trim (command);

    std::cout << command << std::endl;

    Query query = Query::fromJsonString (command);
    protocol.processQuery (s, hdl, msg, emulator, query);
  };

  ns3::emulator::ConnectionPool connPool (control_port, 9000);

  std::cout << "Starting listeners" << std::endl;

  connPool.startListeners (handler)
          .processEvents ();

  /// If we reach this point the control servers have stopped, that means we can also stop the simulation.

  Simulator::Stop ();

  return 0;
}

