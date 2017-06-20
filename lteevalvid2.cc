#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/wifi-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

#include <fstream>
#include <string.h>

#include "ns3/csma-helper.h"
#include "ns3/evalvid-client-server-helper.h"

#include "ns3/lte-helper.h"
#include "ns3/epc-helper.h"
#include "ns3/core-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lte-module.h"
#include "ns3/point-to-point-helper.h"
#include "ns3/config-store.h"
#include "ns3/gtk-config-store.h"


// Bibliotecas para o FlowMonitor
#include "ns3/gnuplot.h"
#include "ns3/flow-monitor-module.h"
#include <ns3/flow-monitor-helper.h>

NS_LOG_COMPONENT_DEFINE ("EvalvidLTEExample");

using namespace ns3;

void ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon,Gnuplot2dDataset DataSet);
void JitterMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset2);
void DelayMonitor(FlowMonitorHelper *fmHelper, Ptr<FlowMonitor> flowMon, Gnuplot2dDataset Dataset3);

int
main (int argc, char *argv[])
{

  LogComponentEnable ("EvalvidClient", LOG_LEVEL_INFO);
  LogComponentEnable ("EvalvidServer", LOG_LEVEL_INFO);

  Time::SetResolution (Time::NS);

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
  enbNodes.Create(numberOfNodes);
  ueNodes.Create(4);

  NodeContainer all;
  all.Add(ueNodes);

  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  for (uint16_t i = 0; i < 4; i++)
    {
      positionAlloc->Add (Vector(distance * i, 0, 0));
    }

  MobilityHelper mobility; 
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (20.0), //onde inicia no eixo X
                                 "MinY", DoubleValue (20.0), //onde inicia no eixo Y
                                 "DeltaX", DoubleValue (20.0), // Distância entre nós
                                 "DeltaY", DoubleValue (20.0), // Distância entre nós
                                 "GridWidth", UintegerValue (4), // Quantidade de colunas em uma linha
                                 "LayoutType", StringValue ("RowFirst")); // Definindo posições em linha
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (enbNodes);

  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (20.0), //onde inicia no eixo X
                                 "MinY", DoubleValue (40.0), //onde inicia no eixo Y
                                 "DeltaX", DoubleValue (20.0), // Distância entre nós
                                 "DeltaY", DoubleValue (20.0), // Distância entre nós
                                 "GridWidth", UintegerValue (4), // Quantidade de colunas em uma linha
                                 "LayoutType", StringValue ("RowFirst")); // Definindo posições em linha
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install(all);


  // Install LTE Devices to the nodes
  NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
  NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (all);

 
  // Install the IP stack on the UEs
  internet.Install (ueNodes);
  Ipv4InterfaceContainer ueIpIface;
  ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
  // Assign IP address to UEs, and install applications

  Ptr<Node> ueNode = ueNodes.Get (0);
  
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

  
  EvalvidClientHelper client (internetIpIfaces.GetAddress (1),port);
  client.SetAttribute ("ReceiverDumpFilename", StringValue("rd_a01_lte"));
  apps = client.Install (ueNodes.Get(0));
  apps.Start (Seconds (10.0));
  apps.Stop (Seconds (90.0));

  //-------------------------Metodo-Animation-------------------------

      AnimationInterface anim ("2lteevalvid.xml"); // Mandatory
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

  NS_LOG_INFO ("Run Simulation.");
  Simulator::Stop(Seconds(90));
  Simulator::Run ();         
  Simulator::Destroy ();
 
  
  NS_LOG_INFO ("Done.");
  return 0;
  
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
