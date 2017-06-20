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
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"

#include "ns3/csma-helper.h"
#include "ns3/evalvid-client-server-helper.h"

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/lte-module.h"

// Bibliotecas para o FlowMonitor
#include "ns3/gnuplot.h"
#include "ns3/flow-monitor-module.h"
#include <ns3/flow-monitor-helper.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include "ns3/csma-module.h"

using namespace ns3;

void ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon,Gnuplot2dDataset DataSet);
void JitterMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset2);
void DelayMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset3);

NS_LOG_COMPONENT_DEFINE ("Project_4.1");

int
main (int argc, char *argv[])
{
  LogComponentEnable ("EvalvidClient", LOG_LEVEL_INFO);
  LogComponentEnable ("EvalvidServer", LOG_LEVEL_INFO);


  uint16_t numberOfNodes = 1;
  // double simTime = 5.0;
  double distance = 60.0;
  // Inter packet interval in ms
  // double interPacketInterval = 1;
  // double interPacketInterval = 100;
   uint16_t port = 8000;


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
  InternetStackHelper internet;
  internet.Install (remoteHost);

  // Create the Internet
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));   //0.010
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);


  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("1.0.0.0", "255.0.0.0");
  Ipv4InterfaceContainer internetIpIfaces = ipv4h.Assign (internetDevices);
  // interface 0 is localhost, 1 is the p2p device
  // Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

  NodeContainer ueNodes;
  NodeContainer enbNodes;
  NodeContainer slNodes;
  enbNodes.Create(numberOfNodes);
  ueNodes.Create(numberOfNodes);
  
  slNodes.Create(3);
  NodeContainer slNodes0 = NodeContainer (ueNodes, slNodes.Get (0));
  NodeContainer slNodes1 = NodeContainer (ueNodes, slNodes.Get (1));
  NodeContainer slNodes2 = NodeContainer (ueNodes, slNodes.Get (2));


  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numberOfNodes; i++)
    {
      positionAlloc->Add (Vector(distance * i, 0, 0));
    }
  
  MobilityHelper mobility; 
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (10.0), //onde inicia no eixo X
                                 "MinY", DoubleValue (20.0), //onde inicia no eixo Y
                                 "DeltaX", DoubleValue (20.0), // Distância entre nós
                                 "DeltaY", DoubleValue (20.0), // Distância entre nós
                                 "GridWidth", UintegerValue (4), // Quantidade de colunas em uma linha
                                 "LayoutType", StringValue ("RowFirst")); // Definindo posições em linha
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNodes);


  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (20.0), //onde inicia no eixo X
                                 "MinY", DoubleValue (20.0), //onde inicia no eixo Y
                                 "DeltaX", DoubleValue (20.0), // Distância entre nós
                                 "DeltaY", DoubleValue (20.0), // Distância entre nós
                                 "GridWidth", UintegerValue (4), // Quantidade de colunas em uma linha
                                 "LayoutType", StringValue ("RowFirst")); // Definindo posições em linha
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ueNodes);


  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (30.0), //onde inicia no eixo X
                                 "MinY", DoubleValue (5.0), //onde inicia no eixo Y
                                 "DeltaX", DoubleValue (20.0), // Distância entre nós
                                 "DeltaY", DoubleValue (20.0), // Distância entre nós
                                 "GridWidth", UintegerValue (1), // Quantidade de colunas em uma linha
                                 "LayoutType", StringValue ("RowFirst")); // Definindo posições em linha
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (slNodes);

  
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  
  WifiHelper wifi; 
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;

  CsmaHelper csma;
  csma.SetChannelAttribute ("DataRate", DataRateValue (DataRate (5000)));
  csma.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));

  NetDeviceContainer slDev0 = csma.Install (slNodes0);
  NetDeviceContainer slDev1 = csma.Install (slNodes1);
  NetDeviceContainer slDev2 = csma.Install (slNodes2);
  

  InternetStackHelper internet2;
  internet2.Install (slNodes);

  Ssid ssid;
  ssid = Ssid ("network-A");

  phy.Set ("ChannelNumber", UintegerValue (36));
  mac.SetType ("ns3::StaWifiMac",
                "Ssid", SsidValue (ssid),
                "ActiveProbing", BooleanValue (false));
  slDev0 = wifi.Install (phy, mac, slNodes0);
  slDev1 = wifi.Install (phy, mac, slNodes1);
  slDev2 = wifi.Install (phy, mac, slNodes2);

  Ipv4AddressHelper address;
  address.SetBase ("192.168.1.0", "255.255.255.0");
  address.Assign (slDev0);
  address.SetBase ("192.168.2.0", "255.255.255.0");
  address.Assign (slDev1);
  address.SetBase ("192.168.3.0", "255.255.255.0");
  address.Assign (slDev2);

  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

 
  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  // Assign IP address to UEs, and install applications

  Ptr<Node> ueNode = ueNodes.Get (0);
   // Set the default gateway for the UE
  Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
  ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);

  // Attach one UE per eNodeB
  lteHelper->Attach (ueLteDevs.Get(0), enbLteDevs.Get(0));

  //lteHelper->ActivateEpsBearer (ueLteDevs, EpsBearer (EpsBearer::NGBR_VIDEO_TCP_DEFAULT), EpcTft::Default ());
  
  NS_LOG_INFO ("Create Applications.");
  
  EvalvidServerHelper server(port);
  server.SetAttribute ("SenderTraceFilename", StringValue("st_highway_cif.st"));
  server.SetAttribute ("SenderDumpFilename", StringValue("sd_a01_lte"));
  ApplicationContainer apps = server.Install(remoteHostContainer.Get(0));
  apps.Start (Seconds (9.0));
  apps.Stop (Seconds (101.0));
  
  EvalvidClientHelper client (internetIpIfaces.GetAddress (1),port);
  client.SetAttribute ("ReceiverDumpFilename", StringValue("rd_a01_lte"));
  apps = client.Install (ueNodes.Get(0));
  apps.Start (Seconds (10.0));
  apps.Stop (Seconds (90.0));
  

  // RFC 863 discard port ("9") indicates packet should be thrown away
  // by the system.  We allow this silent discard to be overridden
  // by the PacketSink application.
  uint16_t port2  = 9;

  // Create the OnOff application to send UDP datagrams of size
  // 512 bytes (default) at a rate of 500 Kb/s (default) from n0
  OnOffHelper onoff ("ns3::UdpSocketFactory", 
                     Address (InetSocketAddress (Ipv4Address ("255.255.255.255"), port2)));
  onoff.SetConstantRate (DataRate ("500kb/s"));

  ApplicationContainer app2 = onoff.Install (slNodes0);
  // Start the application
  app2.Start (Seconds (1.0));
  app2.Stop (Seconds (10.0));

  // Create an optional packet sink to receive these packets
  PacketSinkHelper sink ("ns3::UdpSocketFactory",
                         Address (InetSocketAddress (Ipv4Address::GetAny (), port2)));
  app2 = sink.Install (slNodes0);
  app2.Add (sink.Install (slNodes1));
  app2.Add (sink.Install (slNodes2));
  app2.Start (Seconds (1.0));
  app2.Stop (Seconds (10.0));




//-------------------------Metodo-Animation-------------------------

      AnimationInterface anim ("Project_4.xml"); // Mandatory
      for (uint32_t i = 0; i < ueNodes.GetN (); ++i)
      {
        anim.UpdateNodeDescription (ueNodes.Get (i), "NODE"); // Optional
        anim.UpdateNodeColor (ueNodes.Get (i), 255, 0, 0); // Coloração
      }
      for (uint32_t i = 0; i < enbNodes.GetN (); ++i)
      {
        anim.UpdateNodeDescription (enbNodes.Get (i), "SERVER"); // Optional
        anim.UpdateNodeColor (enbNodes.Get (i), 0, 255, 0); // Coloração
      }
      anim.EnablePacketMetadata (); // Optional
      for (uint32_t i = 0; i < slNodes.GetN (); ++i)
      {
        anim.UpdateNodeDescription (slNodes.Get (i), "SERVER"); // Optional
        anim.UpdateNodeColor (slNodes.Get (i), 0, 0, 255); // Coloração
      }
      anim.EnablePacketMetadata (); // Optional

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop(Seconds(90));
  Simulator::Run ();         
  Simulator::Destroy ();
  
  
   NS_LOG_INFO ("Done.");
  return 0;

}