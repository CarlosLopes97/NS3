   /* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
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
 */

#include "ns3/core-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
//#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
//LTE
#include <fstream>
#include <string.h>

//#include "ns3/csma-helper.h"
#include "ns3/evalvid-client-server-helper.h"

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/netanim-module.h"


// Network Topology
//
//                 [**********************LTE_Architecture**********************]
//    Wifi 10.1.3.0 
//  *    *    *     *                                     10.1.1.0
//  |    |    |     |     LTE 10.1.2.0                 point-to-point
// n5   n6   n7   ueNode -------------- enbNode   pgw ---------------- remotHost
//                                         |       |
// [********WIFI********]                  *       * 
//                                       LTE 10.1.2.0
 
using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("Project_6");

int 
main (int argc, char *argv[])
{
  bool verbose = true;
  uint32_t nWifi = 3;
  bool tracing = false;

  uint16_t numberOfNodes = 1;
  // double simTime = 5.0;
  double distance = 60.0;
  // Inter packet interval in ms
  // double interPacketInterval = 1;
  // double interPacketInterval = 100;
   uint16_t port = 8000;

  CommandLine cmd;
  //cmd.AddValue ("nCsma", "Number of \"extra\" CSMA nodes/devices");
  cmd.AddValue ("nWifi", "Number of wifi STA devices", nWifi);
  cmd.AddValue ("verbose", "Tell echo applications to log if true", verbose);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);

  cmd.Parse (argc,argv);

  // The underlying restriction of 18 is due to the grid position
  // allocator's configuration; the grid layout will exceed the
  // bounding box if more than 18 nodes are provided.
  if (nWifi > 18)
    {
      std::cout << "nWifi should be 18 or less; otherwise grid layout exceeds the bounding box" << std::endl;
      return 1;
    }

  if (verbose)
    {
      LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
      LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }



  //Configure LTE
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  //Ptr<EpcHelper>  epcHelper = CreateObject<EpcHelper> ();
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");

  Ptr<Node> pgw = epcHelper->GetPgwNode ();


  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);
  
  InternetStackHelper stack;
  stack.Install(remoteHost);
 
  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));   //0.010
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);


  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("10.1.2.0", "255.255.255.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  // Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("10.1.1.0"), Ipv4Mask ("255.255.255.0"), 1);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  enbNodes.Create(numberOfNodes);
  ueNodes.Create(numberOfNodes);

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numberOfNodes; i++)
    {
      positionAlloc->Add (Vector(distance * i, 0, 0));
    }
  MobilityHelper mobility;
  mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
  mobility.SetPositionAllocator(positionAlloc);
  mobility.Install(enbNodes);
  mobility.Install(ueNodes);
  mobility.Install(pgw);
  mobility.Install(remoteHost);

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

 
  // Install the IP stack on the UEs
  stack.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  // Assign IP address to UEs, and install applications

  Ptr<Node> ueNode = ueNodes.Get (0);
   // Set the default gateway for the UE
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  // Attach one UE per eNodeB
  lteHelper->Attach (ueLteDevs.Get(0), enbLteDevs.Get(0));

  

  //Configure WIFI
  NodeContainer wifiStaNodes;
  wifiStaNodes.Create (nWifi);
  NodeContainer wifiApNode = ueNodes;

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi;
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;
  Ssid ssid = Ssid ("ns-3-ssid");
  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid),
               "ActiveProbing", BooleanValue (false));

  NetDeviceContainer staDevices;
  staDevices = wifi.Install (phy, mac, wifiStaNodes);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer apDevices;
  apDevices = wifi.Install (phy, mac, wifiApNode);

  //MobilityHelper mobility;

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-50, 50, -50, 50)));
  mobility.Install (wifiStaNodes);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);

  InternetStackHelper stack1;
  stack1.Install (wifiApNode);
  stack1.Install (wifiStaNodes);

  Ipv4AddressHelper address;

  // address.SetBase ("10.1.1.0", "255.255.255.0");
  // Ipv4InterfaceContainer p2pInterfaces;
  // p2pInterfaces = address.Assign (p2pDevices);

  // address.SetBase ("10.1.2.0", "255.255.255.0");
  // Ipv4InterfaceContainer csmaInterfaces;
  // csmaInterfaces = address.Assign (csmaDevices);

  address.SetBase ("10.1.3.0", "255.255.255.0");
  address.Assign (staDevices);
  address.Assign (apDevices);

  NS_LOG_INFO ("Create Applications.");
  
  EvalvidServerHelper server(port);
  server.SetAttribute ("SenderTraceFilename", StringValue("st_highway_cif.st"));
  server.SetAttribute ("SenderDumpFilename", StringValue("sd_a01_lte"));
  
  ApplicationContainer apps = server.Install(remoteHostContainer.Get(0));
  apps.Start (Seconds (1.0));
  apps.Stop (Seconds (9.0));
  
  EvalvidClientHelper client (internetIpIfaces.GetAddress(1),port);
  client.SetAttribute ("ReceiverDumpFilename", StringValue("rd_a01_lte"));
 
  ApplicationContainer clientApps = client.Install (wifiStaNodes.Get (nWifi - 1));
  clientApps.Start (Seconds (1.0));
  clientApps.Stop (Seconds (9.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();



//Metodo Animation

      AnimationInterface anim ("Project_6.xml"); // Mandatory
      for (uint32_t i = 0; i < ueNodes.GetN(); ++i)
      {
        anim.UpdateNodeDescription (ueNodes.Get(i), "MR NODE"); // Optional
        anim.UpdateNodeColor (ueNodes.Get(i), 255, 0, 0); // Coloração
      }
      for (uint32_t i = 0; i < enbNodes.GetN(); ++i)
      {
        anim.UpdateNodeDescription (enbNodes.Get(i), "LTE"); // Optional
        anim.UpdateNodeColor (enbNodes.Get (i), 0, 255, 0); // Coloração
      }
      anim.EnablePacketMetadata (); // Optional
      for (uint32_t i = 0; i < wifiStaNodes.GetN(); ++i)
      {
        anim.UpdateNodeDescription (wifiStaNodes.Get(i), "SLAVE"); // Optional
        anim.UpdateNodeColor (wifiStaNodes.Get(i), 0, 0, 255); // Coloração
      }
      anim.EnablePacketMetadata (); // Optional


  Simulator::Stop (Seconds (10.0));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}