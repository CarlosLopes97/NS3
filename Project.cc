/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 *
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
 * Author: Carlos Lopes <carloslopesufpa@gmail.com>
 */

//To LTE EVALVID.
#include <fstream>
#include <string.h>

#include "ns3/csma-helper.h"
#include "ns3/evalvid-client-server-helper.h"

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/applications-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/netanim-module.h"
#include "ns3/gtk-config-store.h"
//To WIFI 
#include "ns3/wifi-module.h"
#include "ns3/csma-module.h"
#include "ns3/olsr-routing-protocol.h"
#include "ns3/olsr-helper.h"
#include <iostream>
#include <vector>


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("EvalvidLTEExample");

int
main (int argc, char *argv[])
{
 
  LogComponentEnable ("EvalvidClient", LOG_LEVEL_INFO);
  LogComponentEnable ("EvalvidServer", LOG_LEVEL_INFO);


  //uint16_t numberOfNodes = 1;
  // double simTime = 5.0;
  //double distance = 60.0;
  // Inter packet interval in ms
  // double interPacketInterval = 1;
  // double interPacketInterval = 100;
  uint16_t port = 8000;
  std::string phyMode ("DsssRate1Mbps");
  double rss = -80;  // -dBm

  // disable fragmentation for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2200"));
  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("2200"));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode",
                      StringValue (phyMode));

  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  
  //Ptr<EpcHelper>  epcHelper = CreateObject<EpcHelper> ();
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper->SetEpcHelper (epcHelper);
  lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");

  NodeContainer enbNode;
  enbNode.Create(1);
  NodeContainer ueNodes;
  ueNodes.Create(5);

  NodeContainer mr;
  mr.Add(ueNodes.Get(0));

  // Ptr<Node> pgw = epcHelper->GetPgwNode ();

  // Create a single RemoteHost
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (1);
  
  Ptr<Node> remoteHost = remoteHostContainer.Get(0);
  InternetStackHelper internet;
  internet.Install (remoteHost);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));   //0.010
  NetDeviceContainer internetDevicesS = p2ph.Install (ueNodes, remoteHost);

  InternetStackHelper stack;
  stack.Install (ueNodes);
  stack.Install (enbNode);

  Ipv4AddressHelper address;

  address.SetBase ("1.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer InterS = address.Assign (internetDevicesS);


  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  //Create Mobility
  MobilityHelper mobility; 
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (30.0), //onde inicia no eixo X
                                 "MinY", DoubleValue (20.0), //onde inicia no eixo Y
                                 "DeltaX", DoubleValue (20.0), // Distância entre nós
                                 "DeltaY", DoubleValue (20.0), // Distância entre nós
                                 "GridWidth", UintegerValue (5), // Quantidade de colunas em uma linha
                                 "LayoutType", StringValue ("RowFirst")); // Definindo posições em linha
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ueNodes);
  
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (10.0), //onde inicia no eixo X
                                 "MinY", DoubleValue (20.0), //onde inicia no eixo Y
                                 "DeltaX", DoubleValue (20.0), // Distância entre nós
                                 "DeltaY", DoubleValue (20.0), // Distância entre nós
                                 "GridWidth", UintegerValue (5), // Quantidade de colunas em uma linha
                                 "LayoutType", StringValue ("RowFirst")); // Definindo posições em linha
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNode);

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (10.0), //onde inicia no eixo X
                                 "MinY", DoubleValue (20.0), //onde inicia no eixo Y
                                 "DeltaX", DoubleValue (20.0), // Distância entre nós
                                 "DeltaY", DoubleValue (20.0), // Distância entre nós
                                 "GridWidth", UintegerValue (5), // Quantidade de colunas em uma linha
                                 "LayoutType", StringValue ("RowFirst")); // Definindo posições em linha
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (remoteHost);

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNode);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

  // Install the IP stack on the UEs
  internet.Install (mr);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs.Get(0)));

  // Assign IP address to UEs, and install applications
  Ptr<Node> ueNode = ueNodes.Get(0);
  // Set the default gateway for the UE
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  // Attach one UE per eNodeB
  //lteHelper->Attach (ueLteDevs.Get(0), enbLteDevs.Get(0));

  WifiHelper wifi;

  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // This is one parameter that matters when using FixedRssLossModel
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (0) );
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  // The below FixedRssLossModel will cause the rss to be fixed regardless
  // of the distance between the two stations, and the transmit power
  wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a mac and disable rate control
  WifiMacHelper wifiMac;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));



  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, ueNodes);

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000000)));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  NetDeviceContainer csmaDevices = csma.Install (NodeContainer (ueNodes));

  //Create Applications
  NS_LOG_INFO ("Create Applications.");
  EvalvidServerHelper server(port);
  server.SetAttribute ("SenderTraceFilename", StringValue("st_highway_cif.st"));
  server.SetAttribute ("SenderDumpFilename", StringValue("sd_a01_lte"));
  ApplicationContainer apps = server.Install(remoteHostContainer.Get(0));
  apps.Start (Seconds (9.0));
  apps.Stop (Seconds (101.0));
  
  EvalvidClientHelper client (internetDevicesMR.GetAddress (1),port);
  client.SetAttribute ("ReceiverDumpFilename", StringValue("rd_a01_lte"));
  apps = client.Install (ueNodes.Get(0));
  apps.Start (Seconds (10.0));
  apps.Stop (Seconds (90.0));

   //-------------------------Metodo-Animation-------------------------

      AnimationInterface anim ("Project.xml"); // Mandatory
      for (uint32_t i = 0; i < ueNodes.GetN (); ++i)
      {
        anim.UpdateNodeDescription (ueNodes.Get (i), "NODE"); // Optional
        anim.UpdateNodeColor (ueNodes.Get (i), 255, 0, 0); // Coloração
      }
      for (uint32_t i = 0; i < enbNode.GetN (); ++i)
      {
        anim.UpdateNodeDescription (enbNode.Get (i), "SERVER"); // Optional
        anim.UpdateNodeColor (enbNode.Get (i), 0, 255, 0); // Coloração
      }
      anim.EnablePacketMetadata (); // Optional

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop(Seconds(90));
  Simulator::Run ();         
  Simulator::Destroy ();
    
  NS_LOG_INFO ("Done.");
  return 0;
}