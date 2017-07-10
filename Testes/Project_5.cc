/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2016
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
 * AUTOR: Joahannes Costa <joahannes@gmail.com>.
 * EDIT: Leonardo Silva <ti.leonardosilva@gmail.com>.
 * 
 */

#include <iostream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include "ns3/double.h"
#include <ns3/boolean.h>
#include <ns3/enum.h>
#include <iomanip>
#include <string>
#include "ns3/netanim-module.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/evalvid-client-server-helper.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/olsr-module.h"
#include "ns3/config-store-module.h"

//Monitor de fluxo
#include "ns3/flow-monitor-module.h"
#include "ns3/flow-monitor.h"
#include "ns3/flow-monitor-helper.h"
#include "ns3/gnuplot.h"

//LTE
#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/lte-module.h"

//NetAnim
#include "ns3/netanim-module.h"


using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("MCC");


// Métodos QOS

void ThroughputMonitor (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> monitor, Gnuplot2dDataset dataset){

  double tempThroughput = 0.0;
  monitor->CheckForLostPackets(); 
  std::map<FlowId, FlowMonitor::FlowStats> flowStats = monitor->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats){ 
      tempThroughput = (stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstTxPacket.GetSeconds())/1024/1024);
      
      //if(stats->first == 11) {
      dataset.Add((double)Simulator::Now().GetSeconds(), (double)tempThroughput);
      //}
  }
  
  //Tempo que será iniciado
  Simulator::Schedule(Seconds(1),&ThroughputMonitor, fmhelper, monitor, dataset);
}

void DelayMonitor (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> monitor, Gnuplot2dDataset dataset1){
  
  double delay = 0.0;
  monitor->CheckForLostPackets(); 
  std::map<FlowId, FlowMonitor::FlowStats> flowStats = monitor->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats){ 
      //Ipv4FlowClassifier::FiveTuple fiveTuple = classifier->FindFlow (stats->first);
      delay = stats->second.delaySum.GetSeconds ();
      
      //if(stats->first == 11) {
      dataset1.Add((double)Simulator::Now().GetSeconds(), (double)delay);
      //}    
    }
  
  //Tempo que será iniciado
  Simulator::Schedule(Seconds(1),&DelayMonitor, fmhelper, monitor, dataset1);
}

void LostPacketsMonitor (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> monitor, Gnuplot2dDataset dataset2){
  
  double packets = 0.0;
  monitor->CheckForLostPackets(); 
  std::map<FlowId, FlowMonitor::FlowStats> flowStats = monitor->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats){ 
      //Ipv4FlowClassifier::FiveTuple fiveTuple = classifier->FindFlow (stats->first);
      packets = stats->second.lostPackets;
      
      //if(stats->first == 2) {
      dataset2.Add((double)Simulator::Now().GetSeconds(), (double)packets);
      //}
    }
  
  //Tempo que será iniciado
  Simulator::Schedule(Seconds(1),&LostPacketsMonitor, fmhelper, monitor, dataset2);
}

void JitterMonitor (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> monitor, Gnuplot2dDataset dataset3){
  
  double jitter = 0.0;
  monitor->CheckForLostPackets(); 
  std::map<FlowId, FlowMonitor::FlowStats> flowStats = monitor->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());

  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats){ 
      //Ipv4FlowClassifier::FiveTuple fiveTuple = classifier->FindFlow (stats->first);
      jitter = stats->second.jitterSum.GetSeconds ();
      
      //if(stats->first == 2) {
      dataset3.Add((double)Simulator::Now().GetSeconds(), (double)jitter);
      //}
    }
  
  //Tempo que será iniciado
  Simulator::Schedule(Seconds(1),&LostPacketsMonitor, fmhelper, monitor, dataset3);
}

void ImprimeMetricas (FlowMonitorHelper* fmhelper, Ptr<FlowMonitor> monitor){
  double tempThroughput = 0.0;
  monitor->CheckForLostPackets(); 
  std::map<FlowId, FlowMonitor::FlowStats> flowStats = monitor->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());
       
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats){ 
      // A tuple: Source-ip, destination-ip, protocol, source-port, destination-port
      Ipv4FlowClassifier::FiveTuple fiveTuple = classifier->FindFlow (stats->first);
      
      std::cout<<"Flow ID: " << stats->first <<" ; "<< fiveTuple.sourceAddress <<" -----> "<<fiveTuple.destinationAddress<<std::endl;
      std::cout<<"Tx Packets = " << stats->second.txPackets<<std::endl;
      std::cout<<"Rx Packets = " << stats->second.rxPackets<<std::endl;
      std::cout<<"Duration: " <<stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstTxPacket.GetSeconds()<<std::endl;
      std::cout<<"Last Received Packet: "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<std::endl;
      tempThroughput = (stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds() - stats->second.timeFirstTxPacket.GetSeconds())/1024/1024);
      std::cout<<"Throughput: "<< tempThroughput <<" Mbps"<<std::endl;
      std::cout<< "Delay: " << stats->second.delaySum.GetSeconds () << std::endl;
      std::cout<< "LostPackets: " << stats->second.lostPackets << std::endl;
      std::cout<< "Jitter: " << stats->second.jitterSum.GetSeconds () << std::endl;
      //std::cout<<"Last Received Packet: "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds ---->" << "Throughput: " << tempThroughput << " Kbps" << std::endl;
      std::cout<<"------------------------------------------"<<std::endl;
    }
  
  //Tempo que será iniciado
  Simulator::Schedule(Seconds(1),&ImprimeMetricas, fmhelper, monitor);
}


//Main
int main (int argc, char *argv[]){
//Variáveis
  uint16_t simu = 117; //Mudança no seed / run
  uint16_t nUe = 1; //Quantidade de UEs
  uint16_t nEnb = 1; //Quantidade de ENBs
  uint16_t nRemote = 1; //Host remoto
  bool tracing = true; //Rastreamento
  double simTime = 120.0; //Tempo de simulação
  uint16_t numberOfNodes = 1;
  // double simTime = 5.0;
  double distance = 60.0;
  // Inter packet interval in ms
  // double interPacketInterval = 1;
  // double interPacketInterval = 100;
   //uint16_t port = 8000;


//Linha de comando
  CommandLine cmd;
  cmd.AddValue ("simu", "Number of simulation", simu);

  cmd.AddValue ("nUe", "Number of UE nodes", nUe);
  cmd.AddValue ("nEnb", "Number of Enb", nEnb);
  cmd.AddValue ("tracing", "Enable pcap tracing", tracing);
  cmd.AddValue ("simTime", "Time of Simulation", simTime);

  cmd.Parse (argc,argv);


// Mudança do Seed
  ns3::SeedManager::SetSeed(simu);  
  ns3::SeedManager::SetRun(simu);

//LOG's
    LogComponentEnable ("EvalvidClient", LOG_LEVEL_INFO);
    LogComponentEnable ("EvalvidServer", LOG_LEVEL_INFO);

  //LTE Helpers
  Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
  Ptr<PointToPointEpcHelper>  epcHelper = CreateObject<PointToPointEpcHelper> ();
  lteHelper -> SetEpcHelper (epcHelper);
  lteHelper -> SetSchedulerType("ns3::PfFfMacScheduler");
   
  ConfigStore inputConfig;
  inputConfig.ConfigureDefaults ();

//PGW - Packet Data Network Gateway
  Ptr<Node> pgw = epcHelper->GetPgwNode ();

//Remote Host
  NodeContainer remoteHostContainer;
  remoteHostContainer.Create (nRemote); 
  Ptr<Node> remoteHost = remoteHostContainer.Get (0);

//Pilha de Internet
  InternetStackHelper internet;
  internet.Install (remoteHost);

//P2P Interno
  PointToPointHelper p2ph;
  p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
  p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
  p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
  NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);


  //Determina endereço ip para o Link
  Ipv4AddressHelper ipv4h;
  ipv4h.SetBase ("10.1.0.0", "255.255.0.0");
  Ipv4InterfaceContainer internetIpIfaces;
  internetIpIfaces = ipv4h.Assign (internetDevices);

//interface 0 é localhost e interface 1 é dispositivo p2p
 // Ipv4Address remoteHostAddr = internetIpIfaces.GetAddress (1);
  Ipv4StaticRoutingHelper ipv4RoutingHelper;
  Ptr<Ipv4StaticRouting> remoteHostStaticRouting = ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
  remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);

/*-------------------------------------------------------*/

//Cria UEs
  NodeContainer ueNodes;
  ueNodes.Create (nUe);
//Cria ENBs
  NodeContainer enbNodes;
  enbNodes.Create (nEnb);
//Cria WiFiNodes
  NodeContainer wifiNodes = ueNodes;
//Instala pilha de internet nos UEs
  internet.Install(ueNodes);


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
  mobility.Install (wifiNodes);

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



  //Instala LTE Devices para cada grupo de nós
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);

//IPV4 para UEs
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));

//Definir endereços IPs e instala aplicação
  for (uint32_t u = 0; u < ueNodes.GetN(); ++u){
      Ptr<Node> ueNode = ueNodes.Get (u);
      Ptr<Ipv4StaticRouting> ueStaticRouting = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
      ueStaticRouting->SetDefaultRoute (epcHelper->GetUeDefaultGatewayAddress (), 1);
    }

//Anexa as UEs na ENB
  for (uint16_t i = 0; i < ueNodes.GetN(); i++){
      lteHelper->Attach (ueLteDevs.Get(i), enbLteDevs.Get(0));
  }

/*------------------------- WIFI  ----------------------*/

//Cria os nós WiFi e os interliga por meio de um canal
  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

//Algoritmo de controle de taxa
  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211a);

//Camada MAC
  NqosWifiMacHelper mac;
  mac.SetType ("ns3::AdhocWifiMac");

//Nós WiFi
  NetDeviceContainer wifiDevices;
  wifiDevices = wifi.Install (phy, mac, wifiNodes);

//AdHoc
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ipAdhoc = ipv4.Assign (wifiDevices);


  NS_LOG_INFO ("Create Applications.");

//EvalVid 
    for (uint32_t i = 0; i < 1; ++i){

    
        uint16_t port = 1005;
        uint16_t v_port = port * i + 1005;

  EvalvidServerHelper server(v_port);
  server.SetAttribute ("SenderTraceFilename", StringValue("st_highway_cif.st"));
  server.SetAttribute ("SenderDumpFilename", StringValue("sd_a01_lte"));
  ApplicationContainer apps = server.Install(enbNodes.Get(0));
  apps.Start (Seconds (9.0));
  apps.Stop (Seconds (101.0));
  
  EvalvidClientHelper client (ipAdhoc.GetAddress (1), v_port);
  client.SetAttribute ("ReceiverDumpFilename", StringValue("rd_a01_lte"));
  apps = client.Install (ueNodes.Get(0));
  apps.Start (Seconds (10.0));
  apps.Stop (Seconds (90.0));
  }

/*---------------------- Registros ----------------------*/

//Monitor de fluxo
  Ptr<FlowMonitor> monitor;
  FlowMonitorHelper fmhelper;   
  monitor = fmhelper.InstallAll();

// captura QOS
  string tipo = "resultados/cenario2/graficos/lte_";


    //Throughput
    string vazao = tipo + "FlowVSThroughput";
    string graphicsFileName        = vazao + ".png";
    string plotFileName            = vazao + ".plt";
    string plotTitle               = "Flow vs Throughput";
    string dataTitle               = "Throughput";

    Gnuplot gnuplot (graphicsFileName);
    gnuplot.SetTitle (plotTitle);
    gnuplot.SetTerminal ("png");
    gnuplot.SetLegend ("Flow", "Throughput");

    Gnuplot2dDataset dataset;
    dataset.SetTitle (dataTitle);
    dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

    //Delay
    string delay = tipo + "FlowVSDelay";
    string graphicsFileName1        = delay + ".png";
    string plotFileName1            = delay + ".plt";
    string plotTitle1               = "Flow vs Delay";
    string dataTitle1               = "Delay";

    Gnuplot gnuplot1 (graphicsFileName1);
    gnuplot1.SetTitle (plotTitle1);
    gnuplot1.SetTerminal ("png");
    gnuplot1.SetLegend ("Flow", "Delay");

    Gnuplot2dDataset dataset1;
    dataset1.SetTitle (dataTitle1);
    dataset1.SetStyle (Gnuplot2dDataset::LINES_POINTS);

    //LostPackets
    string lost = tipo + "FlowVSLostPackets";
    string graphicsFileName2        = lost + ".png";
    string plotFileName2            = lost + ".plt";
    string plotTitle2               = "Flow vs LostPackets";
    string dataTitle2               = "LostPackets";

    Gnuplot gnuplot2 (graphicsFileName2);
    gnuplot2.SetTitle (plotTitle2);
    gnuplot2.SetTerminal ("png");
    gnuplot2.SetLegend ("Flow", "LostPackets");

    Gnuplot2dDataset dataset2;
    dataset2.SetTitle (dataTitle2);
    dataset2.SetStyle (Gnuplot2dDataset::LINES_POINTS);

    //Jitter
    string jitter = tipo + "FlowVSJitter";
    string graphicsFileName3        = jitter + ".png";
    string plotFileName3            = jitter + ".plt";
    string plotTitle3               = "Flow vs Jitter";
    string dataTitle3               = "Jitter";

    Gnuplot gnuplot3 (graphicsFileName3);
    gnuplot3.SetTitle (plotTitle3);
    gnuplot3.SetTerminal ("png");
    gnuplot3.SetLegend ("Flow", "Jitter");

    Gnuplot2dDataset dataset3;
    dataset3.SetTitle (dataTitle3);
    dataset3.SetStyle (Gnuplot2dDataset::LINES_POINTS);

    //Chama classe de captura do fluxo
    ThroughputMonitor (&fmhelper, monitor, dataset);
    DelayMonitor (&fmhelper, monitor, dataset1);
    LostPacketsMonitor (&fmhelper, monitor, dataset2);
    JitterMonitor (&fmhelper, monitor, dataset3);

//Captura de pacotes
  //phy.EnablePcap ("video/resultados/lte_wifi/wifi_lte", apDevices.Get (0));
  phy.EnablePcapAll ("resultados/cenario2/dados/lte_");



//-------------------------Metodo-Animation-------------------------

      AnimationInterface anim ("Project_5.xml"); // Mandatory
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
      for (uint32_t i = 0; i < wifiNodes.GetN(); ++i)
      {
        anim.UpdateNodeDescription (wifiNodes.Get(i), "SLAVE"); // Optional
        anim.UpdateNodeColor (wifiNodes.Get(i), 0, 0, 255); // Coloração
      }
      anim.EnablePacketMetadata (); // Optional

  Simulator::Stop (Seconds (simTime));

// Simulation RUN

  Simulator::Run ();


ImprimeMetricas (&fmhelper, monitor);

    //Throughput
    gnuplot.AddDataset (dataset);
    std::ofstream plotFile (plotFileName.c_str()); // Abre o arquivo.
    gnuplot.GenerateOutput (plotFile);    //Escreve no arquivo.
    plotFile.close ();        // fecha o arquivo.
    //Delay
    gnuplot1.AddDataset (dataset1);
    std::ofstream plotFile1 (plotFileName1.c_str()); // Abre o arquivo.
    gnuplot1.GenerateOutput (plotFile1);    //Escreve no arquivo.
    plotFile1.close ();        // fecha o arquivo.
    //LostPackets
    gnuplot2.AddDataset (dataset2);
    std::ofstream plotFile2 (plotFileName2.c_str()); // Abre o arquivo.
    gnuplot2.GenerateOutput (plotFile2);    //Escreve no arquivo.
    plotFile2.close ();        // fecha o arquivo.
    //Jitter
    gnuplot3.AddDataset (dataset3);
    std::ofstream plotFile3 (plotFileName3.c_str()); // Abre o arquivo.
    gnuplot3.GenerateOutput (plotFile3);    //Escreve no arquivo.
    plotFile3.close ();        // fecha o arquivo.



//Cria arquivo para Monitor de fluxo
  monitor->SerializeToXmlFile("resultados/cenario2/dados/flow.xml", true, true);

  Simulator::Destroy ();
  return 0;
}
