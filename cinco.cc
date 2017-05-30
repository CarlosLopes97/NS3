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

// Bibliotecas para o FlowMonitor
#include "ns3/gnuplot.h"
#include "ns3/flow-monitor-module.h"
#include <ns3/flow-monitor-helper.h>

using namespace ns3;

void ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon,Gnuplot2dDataset DataSet);
void JitterMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset2);
void DelayMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset3);

NS_LOG_COMPONENT_DEFINE ("FirstScriptExample");

int
main (int argc, char *argv[])
{
  CommandLine cmd;
  cmd.Parse (argc, argv);
  bool Random = false; 
  double speed = 5;
  
  Time::SetResolution (Time::NS);
  LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
  LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Mode", StringValue ("Time"));
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Time", StringValue ("2s"));
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
  Config::SetDefault ("ns3::RandomWalk2dMobilityModel::Bounds", StringValue ("0|200|0|200"));


  NodeContainer server;
  server.Create (1);

  NodeContainer nodes;
  nodes.Create (16);

  // A-B-C-D

  NodeContainer cd; 
  cd.Add(nodes.Get(3)); // Cliente no D node.Get(3);
  cd.Add(nodes.Get(2));

  NodeContainer cb;
  cb.Add(nodes.Get(1));
  cb.Add(nodes.Get(2));

  NodeContainer ba;
  ba.Add(nodes.Get(0));
  ba.Add(nodes.Get(1));

  NodeContainer aServer;
  aServer.Add(server.Get(0));
  aServer.Add(nodes.Get(0));

  // I-F-D

  NodeContainer df;
  df.Add(nodes.Get(3));
  df.Add(nodes.Get(5));

  NodeContainer fi;
  fi.Add(nodes.Get(5));
  fi.Add(nodes.Get(8));

  NodeContainer iServer;
  iServer.Add(server.Get(0));
  iServer.Add(nodes.Get(8));  

  //

  NodeContainer dg;
  dg.Add(nodes.Get(3));
  dg.Add(nodes.Get(6));

  NodeContainer gj;
  gj.Add(nodes.Get(6));
  gj.Add(nodes.Get(9));

  NodeContainer jm;
  jm.Add(nodes.Get(9));
  jm.Add(nodes.Get(12));

  NodeContainer mServer;
  iServer.Add(server.Get(0));
  mServer.Add(nodes.Get(12));

// ----------------------------------------------------

  NodeContainer all;
  
  all.Add(server);
  all.Add(nodes);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  WifiHelper wifi; 
  wifi.SetRemoteStationManager ("ns3::AarfWifiManager");

  WifiMacHelper mac;

  PointToPointHelper pointToPoint;



  NetDeviceContainer Devcd;
  NetDeviceContainer Devcb;
  NetDeviceContainer Devba;
  NetDeviceContainer DevaServer; 
  NetDeviceContainer Devdf;
  NetDeviceContainer Devfi;
  NetDeviceContainer DeviServer;
  NetDeviceContainer Devdg;
  NetDeviceContainer Devgj;
  NetDeviceContainer Devjm;
  NetDeviceContainer DevmServer;

  Ssid ssid;
  ssid = Ssid ("network-A");

  // A-B-C-D

  phy.Set ("ChannelNumber", UintegerValue (36));
  mac.SetType ("ns3::StaWifiMac",
                "Ssid", SsidValue (ssid),
                "ActiveProbing", BooleanValue (false));
  Devcd = wifi.Install (phy, mac, cd);
  Devcb = wifi.Install (phy, mac, cb);
  Devba = wifi.Install (phy, mac, ba);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
  DevaServer = wifi.Install (phy, mac, aServer);


  // I-D-F

  phy.Set ("ChannelNumber", UintegerValue (36));
  mac.SetType ("ns3::StaWifiMac",
                "Ssid", SsidValue (ssid),
                "ActiveProbing", BooleanValue (false));
  Devdf = wifi.Install (phy, mac, df);
  Devfi = wifi.Install (phy, mac, fi);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
  DeviServer = wifi.Install (phy, mac, iServer);
  


// M-J-G-D

  phy.Set ("ChannelNumber", UintegerValue (36));
  mac.SetType ("ns3::StaWifiMac",
                "Ssid", SsidValue (ssid),
                "ActiveProbing", BooleanValue (false));
  Devdg = wifi.Install (phy, mac, dg);
  Devgj = wifi.Install (phy, mac, gj);
  Devjm = wifi.Install (phy, mac, jm);

  mac.SetType ("ns3::ApWifiMac",
               "Ssid", SsidValue (ssid));
  DevmServer = wifi.Install (phy, mac, mServer);

// A-B-C-D
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));
  
  Devcd = pointToPoint.Install (cd);
  Devcb = pointToPoint.Install (cb);
  Devba = pointToPoint.Install (ba);
  DevaServer = pointToPoint.Install (aServer);

// I-D-F
  Devdf = pointToPoint.Install (df);
  Devfi = pointToPoint.Install (fi);
  DeviServer = pointToPoint.Install (iServer);

// M-J-G-D

  Devdg = pointToPoint.Install (dg);
  Devgj = pointToPoint.Install (gj);
  Devjm = pointToPoint.Install (jm);
  DevmServer = pointToPoint.Install (mServer);


  MobilityHelper mobility; 
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0), //onde inicia no eixo X
                                 "MinY", DoubleValue (0.0), //onde inicia no eixo Y
                                 "DeltaX", DoubleValue (20.0), // Distância entre nós
                                 "DeltaY", DoubleValue (20.0), // Distância entre nós
                                 "GridWidth", UintegerValue (4), // Quantidade de colunas em uma linha
                                 "LayoutType", StringValue ("RowFirst")); // Definindo posições em linha
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);

if (Random) {
  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                           "Mode", StringValue ("Time"),
                           "Time", StringValue ("2s"),
                           "Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
                           "Bounds", StringValue ("0|200|0|200"));
}
  mobility.SetMobilityModel ("ns3::ConstantVelocityMobilityModel");
  mobility.Install (server);
  server.Get (0)->GetObject<MobilityModel> ()->SetPosition (Vector (0, 50, 10));
  server.Get (0)->GetObject<ConstantVelocityMobilityModel> ()->SetVelocity (Vector (speed, 0, 0));

  InternetStackHelper stack;
  stack.Install (all);

  Ipv4AddressHelper address;
  
  //A-B-C-D

  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer Intercd = address.Assign (Devcd);

  address.SetBase ("10.1.2.0", "255.255.255.0");

  Ipv4InterfaceContainer Intercb = address.Assign (Devcb);

  address.SetBase ("10.1.3.0", "255.255.255.0");

  Ipv4InterfaceContainer Interba = address.Assign (Devba);

  address.SetBase ("10.1.4.0", "255.255.255.0");

  Ipv4InterfaceContainer InteraServer = address.Assign (DevaServer);

  //I-F-D

  address.SetBase ("10.1.5.0", "255.255.255.0");

  Ipv4InterfaceContainer Interdf = address.Assign (Devdf);

  address.SetBase ("10.1.6.0", "255.255.255.0");

  Ipv4InterfaceContainer Interfi = address.Assign (Devfi);

  address.SetBase ("10.1.7.0", "255.255.255.0");

  Ipv4InterfaceContainer InteriServer = address.Assign (DeviServer);

  //M-J-G-D

  address.SetBase ("10.1.8.0", "255.255.255.0");

  Ipv4InterfaceContainer Interdg = address.Assign (Devdg);

  address.SetBase ("10.1.9.0", "255.255.255.0");

  Ipv4InterfaceContainer Intergj = address.Assign (Devgj);

  address.SetBase ("10.1.10.0", "255.255.255.0");

  Ipv4InterfaceContainer Interjm = address.Assign (Devjm);

  address.SetBase ("10.1.11.0", "255.255.255.0");

  Ipv4InterfaceContainer IntermServer = address.Assign (DevmServer);


  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (server.Get (0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (InteraServer.GetAddress (0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (200));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (0.1)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get(3)); // D
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (12.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  pointToPoint.EnablePcapAll ("tres", false);



//-----------------FlowMonitor-THROUGHPUT----------------

    std::string fileNameWithNoExtension = "FlowVSThroughput_";
    std::string graphicsFileName        = fileNameWithNoExtension + ".png";
    std::string plotFileName            = fileNameWithNoExtension + ".plt";
    std::string plotTitle               = "Flow vs Throughput";
    std::string dataTitle               = "Throughput";

    // Instantiate the plot and set its title.
    Gnuplot gnuplot (graphicsFileName);
    gnuplot.SetTitle (plotTitle);

    // Make the graphics file, which the plot file will be when it
    // is used with Gnuplot, be a PNG file.
    gnuplot.SetTerminal ("png");

    // Set the labels for each axis.
    gnuplot.SetLegend ("Flow", "Throughput");

     
   Gnuplot2dDataset dataset;
   dataset.SetTitle (dataTitle);
   dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);

  //flowMonitor declaration
  FlowMonitorHelper fmHelper;
  Ptr<FlowMonitor> allMon = fmHelper.InstallAll();
  // call the flow monitor function
  ThroughputMonitor(&fmHelper, allMon, dataset); 

//-----------------FlowMonitor-JITTER--------------------

    std::string fileNameWithNoExtension2 = "FlowVSJitter_";
    std::string graphicsFileName2      = fileNameWithNoExtension2 + ".png";
    std::string plotFileName2        = fileNameWithNoExtension2 + ".plt";
    std::string plotTitle2           = "Flow vs Jitter";
    std::string dataTitle2           = "Jitter";

    Gnuplot gnuplot2 (graphicsFileName2);
    gnuplot2.SetTitle(plotTitle2);

  gnuplot2.SetTerminal("png");

  gnuplot2.SetLegend("Flow", "Jitter");

  Gnuplot2dDataset dataset2;
  dataset2.SetTitle(dataTitle2);
  dataset2.SetStyle(Gnuplot2dDataset::LINES_POINTS);

  //FlowMonitorHelper fmHelper;
  //Ptr<FlowMonitor> allMon = fmHelper.InstallAll();

  JitterMonitor(&fmHelper, allMon, dataset2);

//-----------------FlowMonitor-DELAY---------------------

  std::string fileNameWithNoExtension3 = "FlowVSDelay_";
  std::string graphicsFileName3      = fileNameWithNoExtension3 + ".png";
  std::string plotFileName3        = fileNameWithNoExtension3 + ".plt";
  std::string plotTitle3       = "Flow vs Delay";
  std::string dataTitle3       = "Delay";

      Gnuplot gnuplot3 (graphicsFileName3);
      gnuplot3.SetTitle(plotTitle3);

      gnuplot3.SetTerminal("png");

      gnuplot3.SetLegend("Flow", "Delay");

      Gnuplot2dDataset dataset3;
      dataset3.SetTitle(dataTitle3);
      dataset3.SetStyle(Gnuplot2dDataset::LINES_POINTS);

      DelayMonitor(&fmHelper, allMon, dataset3);


//-------------------------Metodo-Animation-------------------------

      AnimationInterface anim ("cinco.xml"); // Mandatory
      for (uint32_t i = 0; i < nodes.GetN (); ++i)
      {
        anim.UpdateNodeDescription (nodes.Get (i), "NODE"); // Optional
        anim.UpdateNodeColor (nodes.Get (i), 255, 0, 0); // Coloração
      }
      for (uint32_t i = 0; i < server.GetN (); ++i)
      {
        anim.UpdateNodeDescription (server.Get (i), "SERVER"); // Optional
        anim.UpdateNodeColor (server.Get (i), 0, 255, 0); // Coloração
      }
      anim.EnablePacketMetadata (); // Optional

  Simulator::Stop (Seconds (15.0));
  Simulator::Run ();
  
//Gnuplot ...continued
  gnuplot.AddDataset (dataset);
  // Open the plot file.
  std::ofstream plotFile (plotFileName.c_str());
  // Write the plot file.
  gnuplot.GenerateOutput (plotFile);
  // Close the plot file.
  plotFile.close ();

//---ContJITTER---

  gnuplot2.AddDataset(dataset2);;
  std::ofstream plotFile2 (plotFileName2.c_str());
  gnuplot2.GenerateOutput(plotFile2);
  plotFile2.close();

  //---ContDelay---

  gnuplot3.AddDataset(dataset3);;
  std::ofstream plotFile3 (plotFileName3.c_str());
  gnuplot3.GenerateOutput(plotFile3);
  plotFile3.close();


  // GtkConfigStore config;
  // config.ConfigureAttributes ();

  Simulator::Destroy ();
  return 0;
}
//-------------------------Metodo-VAZÃO---------------------------

void ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon,Gnuplot2dDataset DataSet)
  {
        double localThrou=0;
    std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
    Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());
    for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
    {
                      if(stats->first == 2){//IFFFFFFFFFFFFFFFFFFFFFFF
      Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
      std::cout<<"Flow ID     : " << stats->first <<" ; "<< fiveTuple.sourceAddress <<" -----> "<<fiveTuple.destinationAddress<<std::endl;
      std::cout<<"Tx Packets = " << stats->second.txPackets<<std::endl;
      std::cout<<"Rx Packets = " << stats->second.rxPackets<<std::endl;
            std::cout<<"Duration    : "<<(stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())<<std::endl;
      std::cout<<"Last Received Packet  : "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<std::endl;
      std::cout<<"Throughput: " << stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps"<<std::endl;
            localThrou=(stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1024/1024);
      // updata gnuplot data
            DataSet.Add((double)Simulator::Now().GetSeconds(),(double) localThrou);
      std::cout<<"---------------------------------------------------------------------------"<<std::endl;
  }//IFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF  
}
      Simulator::Schedule(Seconds(1),&ThroughputMonitor, fmhelper, flowMon,DataSet);
   //if(flowToXml)
      {
  flowMon->SerializeToXmlFile ("ThroughputMonitor.xml", true, true);
      }

  }

//-------------------------Metodo-JINTTER-------------------------

double atraso1=0;
void JitterMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset2)
{
       double localJitter=0;
       double atraso2 =0;

       std::map<FlowId, FlowMonitor::FlowStats> flowStats2 = flowMon->GetFlowStats();
       Ptr<Ipv4FlowClassifier> classing2 = DynamicCast<Ipv4FlowClassifier> (fmHelper->GetClassifier());
       for(std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats2 = flowStats2.begin(); stats2 != flowStats2.end(); ++stats2)
       {
               if(stats2->first == 2){//IFFFFFFFFFFF
            Ipv4FlowClassifier::FiveTuple fiveTuple2 = classing2->FindFlow (stats2->first);
    std::cout<<"Flow ID : "<< stats2->first <<";"<< fiveTuple2.sourceAddress <<"------>" <<fiveTuple2.destinationAddress<<std::endl;
    std::cout<<"Tx Packets = " << stats2->second.txPackets<<std::endl;
    std::cout<<"Rx Packets = " << stats2->second.rxPackets<<std::endl;
    std::cout<<"Duration  : "<<(stats2->second.timeLastRxPacket.GetSeconds()-stats2->second.timeFirstTxPacket.GetSeconds())<<std::endl;
    std::cout<<"Atraso: "<<stats2->second.timeLastRxPacket.GetSeconds()-stats2->second.timeLastTxPacket.GetSeconds() <<"s"<<std::endl;
atraso2 = stats2->second.timeLastRxPacket.GetSeconds()-stats2->second.timeLastTxPacket.GetSeconds();
//atraso1 = stats2->second.timeFirstRxPacket.GetSeconds()-stats2->second.timeFirstTxPacket.GetSeconds();
    std::cout<<"Jitter: "<< atraso2-atraso1 <<std::endl;
    localJitter= atraso2-atraso1;//Jitter
    Dataset2.Add((double)Simulator::Now().GetSeconds(), (double) localJitter);
    std::cout<<"---------------------------------------------------------------------------"<<std::endl;
               }//IFFFFFFFFFF

       atraso1 = atraso2;
       }

       Simulator::Schedule(Seconds(1),&JitterMonitor, fmHelper, flowMon, Dataset2);
       {
         flowMon->SerializeToXmlFile("JitterMonitor.xml", true, true);
       }
}

//-------------------------Metodo-DELAY---------------------------

void  DelayMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset3)
{
  double localDelay=0;

  std::map<FlowId, FlowMonitor::FlowStats> flowStats3 = flowMon->GetFlowStats();
  Ptr<Ipv4FlowClassifier> classing3 = DynamicCast<Ipv4FlowClassifier> (fmHelper->GetClassifier());
  for(std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats3 = flowStats3.begin(); stats3 != flowStats3.end(); ++stats3)
  {
               if(stats3->first == 2){//IFFFFFFFFFFF
          Ipv4FlowClassifier::FiveTuple fiveTuple3 = classing3->FindFlow (stats3->first);
          std::cout<<"Flow ID : "<< stats3->first <<";"<< fiveTuple3.sourceAddress <<"------>" <<fiveTuple3.destinationAddress<<std::endl;
          localDelay = stats3->second.timeLastRxPacket.GetSeconds()-stats3->second.timeLastTxPacket.GetSeconds();
        Dataset3.Add((double)Simulator::Now().GetSeconds(), (double) localDelay);
        std::cout<<"---------------------------------------------------------------------------"<<std::endl;
    }//IFFFFFFFFF
  }
  Simulator::Schedule(Seconds(1),&DelayMonitor, fmHelper, flowMon, Dataset3);
  {
     flowMon->SerializeToXmlFile("DelayMonitor.xml", true, true);
  }
}