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

  std::string phyMode ("DsssRate1Mbps");
 
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));

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

  // Install Mobility Model
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < numberOfNodes; i++)
    {
      positionAlloc->Add (Vector(distance * i, 0, 0));
    }
  
  MobilityHelper mobility; 
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (30.0), //onde inicia no eixo X
                                 "MinY", DoubleValue (20.0), //onde inicia no eixo Y
                                 "DeltaX", DoubleValue (20.0), // Distância entre nós
                                 "DeltaY", DoubleValue (20.0), // Distância entre nós
                                 "GridWidth", UintegerValue (4), // Quantidade de colunas em uma linha
                                 "LayoutType", StringValue ("RowFirst")); // Definindo posições em linha
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNodes);


  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (40.0), //onde inicia no eixo X
                                 "MinY", DoubleValue (20.0), //onde inicia no eixo Y
                                 "DeltaX", DoubleValue (20.0), // Distância entre nós
                                 "DeltaY", DoubleValue (20.0), // Distância entre nós
                                 "GridWidth", UintegerValue (4), // Quantidade de colunas em uma linha
                                 "LayoutType", StringValue ("RowFirst")); // Definindo posições em linha
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (ueNodes);


  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (60.0), //onde inicia no eixo X
                                 "MinY", DoubleValue (0.0), //onde inicia no eixo Y
                                 "DeltaX", DoubleValue (20.0), // Distância entre nós
                                 "DeltaY", DoubleValue (20.0), // Distância entre nós
                                 "GridWidth", UintegerValue (1), // Quantidade de colunas em uma linha
                                 "LayoutType", StringValue ("RowFirst")); // Definindo posições em linha
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (slNodes);

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (50.0), //onde inicia no eixo X
                                 "MinY", DoubleValue (20.0), //onde inicia no eixo Y
                                 "DeltaX", DoubleValue (20.0), // Distância entre nós
                                 "DeltaY", DoubleValue (20.0), // Distância entre nós
                                 "GridWidth", UintegerValue (1), // Quantidade de colunas em uma linha
                                 "LayoutType", StringValue ("RowFirst")); // Definindo posições em linha
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (remoteHost);

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (10.0), //onde inicia no eixo X
                                 "MinY", DoubleValue (20.0), //onde inicia no eixo Y
                                 "DeltaX", DoubleValue (20.0), // Distância entre nós
                                 "DeltaY", DoubleValue (20.0), // Distância entre nós
                                 "GridWidth", UintegerValue (1), // Quantidade de colunas em uma linha
                                 "LayoutType", StringValue ("RowFirst")); // Definindo posições em linha
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (pgw);


								WifiHelper wifi;

								YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
								// set it to zero; otherwise, gain will be added
								wifiPhy.Set ("RxGain", DoubleValue (-10) );
								// ns-3 supports RadioTap and Prism tracing extensions for 802.11b
								wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);

								YansWifiChannelHelper wifiChannel;
								wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
								wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
								wifiPhy.SetChannel (wifiChannel.Create ());

								// Add an upper mac and disable rate control
								WifiMacHelper wifiMac;
								wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
								wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
												              "DataMode",StringValue (phyMode),
												              "ControlMode",StringValue (phyMode));
								// Set it to adhoc mode
								wifiMac.SetType ("ns3::AdhocWifiMac");
								NetDeviceContainer slDev = wifi.Install (wifiPhy, wifiMac, slNodes);



				  // Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
				  // //Ptr<EpcHelper>  epcHelper = CreateObject<EpcHelper> ();
				  // Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
				  // lteHelper->SetEpcHelper (epcHelper);
				  // lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");

				  // Ptr<Node> pgw = epcHelper->GetPgwNode ();


				  // Create a single RemoteHost
				  internet.Install (slNodes);
				  Ipv4AddressHelper ipv4h;
				  ipv4h.SetBase ("2.0.0.0", "255.0.0.0");
				  Ipv4InterfaceContainer intersl = ipv4h.Assign (slDev);
				  // interface 0 is localhost, 1 is the p2p device
				  // Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);

				  Ipv4StaticRoutingHelper ipv4RoutingHelper;
				  Ptr<Ipv4StaticRouting> SlNodeStaticRouting = ipv4RoutingHelper.GetStaticRouting (slNodes->GetObject<Ipv4> ());
				  SlNodeStaticRouting->AddNetworkRouteTo (Ipv4Address ("8.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);



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
  ApplicationContainer apps = server.Install(enbNodes.Get(0));
  apps.Start (Seconds (9.0));
  apps.Stop (Seconds (101.0));
  
  EvalvidClientHelper client (intersl.GetAddress (1), port);
  client.SetAttribute ("ReceiverDumpFilename", StringValue("rd_a01_lte"));
  apps = client.Install (ueNodes.Get(0));
  apps.Start (Seconds (10.0));
  apps.Stop (Seconds (90.0));



//-------------------------Metodo-Animation-------------------------

      AnimationInterface anim ("Project_4.xml"); // Mandatory
      for (uint32_t i = 0; i < ueNodes.GetN (); ++i)
      {
        anim.UpdateNodeDescription (ueNodes.Get (i), "MR NODE"); // Optional
        anim.UpdateNodeColor (ueNodes.Get (i), 255, 0, 0); // Coloração
      }
      for (uint32_t i = 0; i < enbNodes.GetN (); ++i)
      {
        anim.UpdateNodeDescription (enbNodes.Get (i), "LTE"); // Optional
        anim.UpdateNodeColor (enbNodes.Get (i), 0, 255, 0); // Coloração
      }
      anim.EnablePacketMetadata (); // Optional
      for (uint32_t i = 0; i < slNodes.GetN (); ++i)
      {
        anim.UpdateNodeDescription (slNodes.Get (i), "SLAVE"); // Optional
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