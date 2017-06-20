#include <fstream>
#include <string.h>
#include "ns3/csma-helper.h"
#include "ns3/csma-net-device.h"
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
#include "ns3/netanim-module.h"
#include "ns3/olsr-module.h"
#include "ns3/ipv4-static-routing-helper.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/pointer.h"
#include <iostream>
#include <vector>
#include <cstdio>
#include <cstdlib>
#include <stdexcept>
#include <ctime>
#include <iomanip>
using namespace ns3;
using namespace std;

NS_LOG_COMPONENT_DEFINE ("LTE-Wifi");
const double PI = 3.14159265358979323846;
const double lambda = (3.0e8 / 2.407e9);
bool useTwoRay = true;
double txPowerDbm = 10;
double factorCca = 1.5;
double distancia = 150;
double height = 1.2;
double stop = 90;
double warmup = 30;
void PrintRoutingTable (Ptr<Node> node);
double rxPowerDbm (double distance, double height, double txPowerDbm, bool useTwoRay);
void CourseChange (std::string context, Ptr<const MobilityModel> model);
void ComputeResults (void);

struct sim_Result
{
	uint64_t sumRxBytesByFlow;
	uint64_t sumRxBytesQuadByFlow;
	uint64_t sumLostPktsByFlow;
	uint64_t sumRxPktsByFlow;
	uint64_t sumTxPktsByFlow;
	uint64_t sumDelayFlow;
	uint64_t nFlows;
	/* Throughput Average by Flow (bps) = sumRxBytesByFlow * 8 / (nFlows * time)
	* Throughput Quadratic Average by Flow (bps) = sumRxBytesQuadByFlow * 64 / (nFlows * time * time)
	* Net Aggregated Throughput Average by Node (bps) = sumRxBytesByFlow * 8 / (nodes * time)
	* Fairness = sumRxBytesByFlowˆ2 / (nFlows * sumRxBytesQuadByFlow)
	* Delay per Packet (seconds/packet) = sumDelayFlow / sumRxPktsByFlow
	* Lost Ratio (%) = 100 * sumLostPktsByFlow / sumTxPktsByFlow
	*/
	double thrpAvgByFlow;
	double thrpAvgQuadByFlow;
	double thrpVarByFlow;
	double netThrpAvgByNode;
	double fairness;
	double delayByPkt;
	double lostRatio;
	double pdr;
	sim_Result ()
		{
		sumRxBytesByFlow = 0;
		sumRxBytesQuadByFlow = 0;
		sumLostPktsByFlow = 0;
		sumRxPktsByFlow = 0;
		sumTxPktsByFlow = 0;
		sumDelayFlow = 0;
		nFlows = 0;
		}
} data;
//=============== COMEÇO DA MAIN ===============
int
main (int argc, char *argv[])
{
//==================== LOG’s ===================
//LogComponentEnable ("OnOffApplication", LOG_LEVEL_INFO);
//LogComponentEnable ("FlowMonitor", LOG_LEVEL_ALL);
//LogComponentEnable ("OlsrRoutingProtocol", LOG_LEVEL_ALL);
//LogComponentEnable ("OlsrRoutingTable", LOG_LEVEL_ALL);
//LogComponentEnable ("Ipv4StaticRouting", LOG_LEVEL_ALL);
//Aparecer no arquivo externo: ./waf –run olsr-lte > log.out 2>&1
//=================== PARÂMETROS ==================
uint16_t numberOfNodes = 1;
uint16_t numWifi = 25;
uint16_t port = 80;
uint32_t packetSize = 1472;
uint32_t numFluxos = 1;
double distance = 10.0;
double RxNoiseFiguredB = 7;
uint32_t dist = 600;
double dataRate = 117760;
//double dataRate_f = (double) dataRate / numFluxos;
FILE *top;
double px,py,pz;
top = fopen("Topologia/Topologia1/Topologia1.txt","r");
double k = 0;
std::string phyMode ("DsssRate1Mbps");
Config::SetDefault ("ns3::ArpCache::PendingQueueSize", UintegerValue (400));
Config::SetDefault ("ns3::LteRlcUm::MaxTxBufferSize", UintegerValue (10000000));
CommandLine cmd;
cmd.AddValue ("numFluxos", "Numero de fluxos", numFluxos);
cmd.AddValue ("dataRate", "Taxa de tráfego", dataRate);
cmd.AddValue ("k", "K", k);
cmd.Parse (argc, argv);
//================== LTE Helpers ==================
Ptr<LteHelper> lteHelper = CreateObject<LteHelper> ();
Ptr<EpcHelper> epcHelper = CreateObject<EpcHelper> ();
lteHelper->SetEpcHelper (epcHelper);
lteHelper->SetSchedulerType("ns3::PfFfMacScheduler");
Ptr<Node> pgw = epcHelper->GetPgwNode ();
//========== LTE Remote Host e p2p interno ==========
// Remoto
NodeContainer remoteHostContainer;
remoteHostContainer.Create (1);
Ptr<Node> remoteHost = remoteHostContainer.Get (0);
// p2p interno
PointToPointHelper p2ph;
p2ph.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
p2ph.SetDeviceAttribute ("Mtu", UintegerValue (1500));
p2ph.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010))); //0.010
NetDeviceContainer internetDevices = p2ph.Install (pgw, remoteHost);
// Internet
InternetStackHelper internet;
internet.Install (remoteHost);
//============ Criação dos nós LTE-p2p-wifi ============
NodeContainer ueNodes;
NodeContainer enbNodes;
enbNodes.Create(numberOfNodes);
ueNodes.Create(numberOfNodes);
NodeContainer p2pNodes;
p2pNodes.Create (1);
p2pNodes.Add(ueNodes.Get(0));
NodeContainer wifiNodes;
wifiNodes.Create (numWifi);
wifiNodes.Add(p2pNodes.Get(0));
NodeContainer allNodes;
allNodes.Add(remoteHostContainer.Get(0));
allNodes.Add(enbNodes.Get(0));
allNodes.Add(ueNodes.Get(0));
allNodes.Add(p2pNodes.Get(0));
// Ponteiros
Ptr<Node> ueNode = ueNodes.Get (0);
// Internet
internet.Install (ueNodes);
OlsrHelper olsr;
Ipv4StaticRoutingHelper ipv4static;
Ipv4ListRoutingHelper list;
list.Add (olsr, 10);
list.Add (ipv4static, 10);
internet.SetRoutingHelper(olsr);
for (uint16_t i = 0; i < numWifi; i++)
{
internet.Install (wifiNodes.Get(i));
allNodes.Add(wifiNodes.Get(i));
}
internet.Reset();
internet.SetRoutingHelper(list);
internet.Install (wifiNodes.Get(numWifi));
//================== Mobilidade ================
Ptr<ListPositionAllocator> positionAllocLTE = CreateObject<ListPositionAllocator> ();
Ptr<ListPositionAllocator> positionAllocWIFI = CreateObject<ListPositionAllocator> ();
//LTE
for (uint16_t i = 0; i < numberOfNodes + 1; i++)
{
positionAllocLTE->Add (Vector(distance * i, 0, 0));
}
//reads node’s position from file
for(int i=0;i<numWifi + 1;i++)
{
fscanf(top,"%lf %lf %lf",&px,&py,&pz);
positionAllocWIFI->Add(Vector(px,py,pz));
}
//WIFI
//Modelo
MobilityHelper mobility;
mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
mobility.SetPositionAllocator(positionAllocLTE);
mobility.Install(enbNodes);
mobility.Install(ueNodes);
mobility.SetPositionAllocator(positionAllocWIFI);
mobility.SetMobilityModel ("ns3::RandomDirection2dMobilityModel",
"Bounds", RectangleValue (Rectangle (0, dist, 0, dist)),
"Speed", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"),
"Pause", StringValue ("ns3::ExponentialRandomVariable[Mean=0.5]"));
mobility.Install(wifiNodes);
//Imprime a posição dos nós
for (int j=numWifi; j>0; j–){
std::ostringstream oss;
oss <<"/NodeList/<< wifiNodes.Get (numWifi - j)->GetId ()" << "/$ns3::MobilityModel/CourseChange";
Config::Connect (oss.str (), MakeCallback (&CourseChange));
}
//mobility tracing
AsciiTraceHelper ascii;
MobilityHelper::EnableAsciiAll (ascii.CreateFileStream ("Topologia/Topologia1/v2/mob-olsr/lte-wifi-OLSR.mob"));
//=============== LTE devices e Ipv4 para o UE =====================
//Devices
NetDeviceContainer enbLteDevs = lteHelper->InstallEnbDevice (enbNodes);
NetDeviceContainer ueLteDevs = lteHelper->InstallUeDevice (ueNodes);
//Ipv4 para o UE
Ipv4InterfaceContainer ueIpIface;
ueIpIface = epcHelper->AssignUeIpv4Address (NetDeviceContainer (ueLteDevs));
//Infra-estrutura
lteHelper->Attach (ueLteDevs.Get(0), enbLteDevs.Get(0));
//=================== Point to Point devices ====================
PointToPointHelper pointToPoint;
pointToPoint.SetDeviceAttribute ("DataRate", DataRateValue (DataRate ("100Gb/s")));
pointToPoint.SetDeviceAttribute ("Mtu", UintegerValue (1500));
pointToPoint.SetChannelAttribute ("Delay", TimeValue (Seconds (0.010)));
NetDeviceContainer p2pDevices = pointToPoint.Install (p2pNodes.Get(0), p2pNodes.Get(1));
//================ Adhoc devices =====================
//********************Wifi*************************
WifiHelper wifi;
wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
/*wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
"DataMode", StringValue ("DsssRate1Mbps"),
"RtsCtsThreshold", UintegerValue (1000));*/
wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
"DataMode",StringValue (phyMode),
"ControlMode",StringValue (phyMode));
// turn off RTS/CTS for frames below 1000 bytes
Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue ("0"));
//****************Camada PHY************************
YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
wifiPhy.Set ("TxPowerStart", DoubleValue(txPowerDbm));
wifiPhy.Set ("TxPowerEnd", DoubleValue(txPowerDbm));
wifiPhy.Set ("TxGain", DoubleValue(0));
wifiPhy.Set ("RxGain", DoubleValue(0));
wifiPhy.Set ("EnergyDetectionThreshold", DoubleValue(rxPowerDbm (distancia, height, txPowerDbm, useTwoRay)));
wifiPhy.Set ("CcaMode1Threshold", DoubleValue(rxPowerDbm (distancia*factorCca, height,txPowerDbm, useTwoRay)));
wifiPhy.Set ("RxNoiseFigure", DoubleValue(RxNoiseFiguredB));
cout << "EnergyDetectionThresold:<< rxPowerDbm (distancia, height,txPowerDbm, useTwoRay) << endl;
cout << "CcaMode1Thresold:<< rxPowerDbm (distancia*factorCca, height,txPowerDbm, useTwoRay) << endl;
wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO);
//******************Channel**************************
YansWifiChannelHelper wifiChannel;
wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
wifiChannel.AddPropagationLoss ("ns3::TwoRayGroundPropagationLossModel",
"Frequency", DoubleValue(2.407e9));
//wifiChannel.AddPropagationLoss ("ns3::FixedRssLossModel","Rss",DoubleValue (rss));
//***************** Desvanecimento *****************//
double m;
m = (pow((k+1),2)) / (2*k+1); // "Wireless Communications"(Molisch)
wifiChannel.AddPropagationLoss ("ns3::NakagamiPropagationLossModel",
"m0", DoubleValue (m),"m1", DoubleValue (m),"m2", DoubleValue (m));
wifiPhy.SetChannel (wifiChannel.Create ());
//*******************Camada MAC***********************
NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default ();
wifiMac.SetType ("ns3::AdhocWifiMac");
//*********************Install************************
NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, wifiNodes);
//NetDeviceContainer deviceC = wifi.Install (wifiPhy, wifiMac, p2pNodes.Get(0));
//=============== Ipv4 restante ======================
Ipv4AddressHelper ipv4;
//RemoteHost
ipv4.SetBase ("1.0.0.0", "255.0.0.0");
Ipv4InterfaceContainer internetIpIfaces = ipv4.Assign (internetDevices);
//ADHOC
ipv4.SetBase ("172.1.1.0", "255.255.255.0");
Ipv4InterfaceContainer i = ipv4.Assign (devices);
//P2P
ipv4.SetBase ("7.1.1.0", "255.255.255.0");
Ipv4InterfaceContainer p2pInterfaces = ipv4.Assign (p2pDevices);
//=============== Roteamento Estático ====================
//Remote Host
Ipv4StaticRoutingHelper ipv4RoutingHelper;
Ptr<Ipv4StaticRouting> remoteHostStaticRouting =
ipv4RoutingHelper.GetStaticRouting (remoteHost->GetObject<Ipv4> ());
remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("7.0.0.0"), Ipv4Mask ("255.0.0.0"), 1);
remoteHostStaticRouting->AddNetworkRouteTo (Ipv4Address ("172.1.1.0"), Ipv4Mask ("255.255.255.0"), 1);
//PGW
Ptr<Ipv4StaticRouting> remoteHostStaticRouting79;
remoteHostStaticRouting79 = ipv4RoutingHelper.GetStaticRouting (pgw->GetObject<Ipv4> ());
remoteHostStaticRouting79->AddNetworkRouteTo (Ipv4Address ("172.1.1.0"),
Ipv4Mask ("255.255.255.0"), Ipv4Address ("0.0.0.0"), 1);
//UE node - (Relay de LTE/p2p)
Ptr<Ipv4StaticRouting> remoteHostStaticRouting2;
remoteHostStaticRouting2 = ipv4RoutingHelper.GetStaticRouting (ueNode->GetObject<Ipv4> ());
remoteHostStaticRouting2->AddNetworkRouteTo Ipv4Address ("1.0.0.0"), Ipv4Mask ("255.0.0.0"),
epcHelper->GetUeDefaultGatewayAddress (), 1;
remoteHostStaticRouting2->AddNetworkRouteTo Ipv4Address ("172.1.1.0"), Ipv4Mask ("255.255.255.0"),
Ipv4Address ("7.1.1.1"), 2;
//p2p(0) - (Relay de p2p/WIFI)
Ptr<Ipv4StaticRouting> remoteHostStaticRouting89;
remoteHostStaticRouting89 = ipv4RoutingHelper.GetStaticRouting (p2pNodes.Get(0)->GetObject<Ipv4> ());
remoteHostStaticRouting89->AddHostRouteTo (Ipv4Address ("1.0.0.2"), Ipv4Address ("7.1.1.2"), 2);
//WIFI
/*Ptr<Ipv4StaticRouting> remoteHostStaticRouting65;
remoteHostStaticRouting65 = ipv4RoutingHelper.GetStaticRouting (wifiNodes.Get(numCliente)->GetObject<Ipv4> ());
remoteHostStaticRouting65->AddHostRouteTo (Ipv4Address ("1.0.0.2"), i.GetAddress (numWifi), 1);*/
//================ Traces e Logs ====================
pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("Topologia/Topologia1/v2/trace-olsr/lte-wifi.tr-OLSR"));
pointToPoint.EnablePcapAll ("Topologia/Topologia1/v2/pcap-p2p-olsr/LTE-WIFI-p2p-OLSR");
wifiPhy.EnablePcap ("Topologia/Topologia1/v2/pcap-olsr/LTE-WIFI-OLSR", devices);
//Ptr<OutputStreamWrapper> stream = ascii.CreateFileStream ("routingtables.tr");
//remoteHostStaticRouting->PrintRoutingTable(stream);
//lteHelper->EnableTraces ();
//====================== Aplicação ========================
for (uint32_t n=19; n<19 +numFluxos; n++)
{
OnOffHelper onoff ("ns3::UdpSocketFactory",Address (InetSocketAddress (i.GetAddress (n), port))); //Destination
onoff.SetAttribute ("Remote", AddressValue(InetSocketAddress(i.GetAddress(n),port))); //Destination
onoff.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=1.0]"));
onoff.SetAttribute ("DataRate", DataRateValue (DataRate (dataRate / numFluxos)));
onoff.SetAttribute("PacketSize", UintegerValue (packetSize));
ApplicationContainer app = onoff.Install (remoteHostContainer.Get(0)); //Source
app.Start (Seconds (warmup+0.1));
app.Stop (Seconds (stop + 0.1));
PacketSinkHelper sink("ns3::UdpSocketFactory",InetSocketAddress(internetIpIfaces.GetAddress(0), port));//Source
ApplicationContainer sinkApp = sink.Install(wifiNodes.Get(n)); //Destination
sinkApp.Start(Seconds(warmup+0.1));
sinkApp.Stop(Seconds(stop + 0.1));
}
//==================== Run e Outputs ========================
PrintRoutingTable(wifiNodes.Get(numWifi));
PrintRoutingTable(pgw);
PrintRoutingTable(enbNodes.Get(0));
PrintRoutingTable(p2pNodes.Get(1));
PrintRoutingTable(p2pNodes.Get(0));
//=================== Flow Monitor ==========================
FlowMonitorHelper flowmon;
Ptr<FlowMonitor> monitor = flowmon.Install(allNodes);
monitor->Start (Seconds (warmup)); // start monitoring after network warm up
monitor->Stop (Seconds (stop + 3)); // stop monitoring
Simulator::Stop(Seconds(stop + 3));
Simulator::Run ();
Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier());
std::map<FlowId, FlowMonitor::FlowStats> stats = monitor->GetFlowStats();
for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); ++i)
{
Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
std::cout << "Flow << i->first << "(<< t.sourceAddress << -> << t.destinationAddress << ")\n";
std::cout << "Tx Bytes: << i->second.txBytes << \n";
std::cout << "Rx Bytes: << i->second.rxBytes << \n";
std::cout << "Throughput: << i->second.rxBytes * 8.0 / (stop - warmup) / 1024 / 1024 << "Mbps\n;
std::cout << "Packet Delivery Ratio: << ((double)(i->second.rxPackets)/(double)(i->second.txPackets))*100 << "%"\n";
if (t.destinationPort == 80) // only http flows
{
data.nFlows++;
data.sumRxBytesByFlow += i->second.rxBytes; // sum flows
data.sumRxBytesQuadByFlow += i->second.rxBytes * i->second.rxBytes; // sum flows2
data.sumDelayFlow += i->second.delaySum.GetInteger (); // sum delays
data.sumRxPktsByFlow += i->second.rxPackets; // sum rx pkts
data.sumTxPktsByFlow += i->second.txPackets; // sum tx pkts
data.sumLostPktsByFlow += i->second.lostPackets; // sum lost pkts
}
}
monitor->SerializeToXmlFile ("Topologia/Topologia1/v2/FlowMonitor/FlowMonitor LteWifi OLSR.xml", true, true);
monitor->CheckForLostPackets();
Simulator::Destroy ();
ComputeResults ();
return 0;
}
double
rxPowerDbm (double distance, double height, double txPowerDbm, bool useTwoRay)
{
double lossPowerDbm;
if (useTwoRay){
double dCross = (4 * PI * height * height) / lambda;
if (distance <= dCross){
lossPowerDbm = 10 * log10( lambda*lambda / (16.0 * PI * PI * distance*distance));
} else {
lossPowerDbm = 10 * log10( (height*height*height*height) / (distance*distance*distance*distance) );
}
}
else {
lossPowerDbm = 10 * log10( lambda*lambda / (16.0 * PI * PI * distance*distance));
}
return txPowerDbm + lossPowerDbm;
}
void
CourseChange (std::string context, Ptr<const MobilityModel> model)
{
Vector position = model->GetPosition ();
NS_LOG_UNCOND (context <<"x = << position.x << ", y = << position.y);
}
void
PrintRoutingTable (Ptr<Node> node)
{
Ipv4StaticRoutingHelper helper;
Ptr<Ipv4> stack = node -> GetObject<Ipv4>();
Ptr<Ipv4StaticRouting> staticrouting = helper.GetStaticRouting(stack);
uint32_t numroutes=staticrouting->GetNRoutes();
Ipv4RoutingTableEntry entry;
std::cout << "Routing table for device: << Names::FindName(node) <<\n";
std::cout << "Destination\t\tMask\t\t\tGateway\t\t\tIface\n";
for (uint32_t i =0 ; i<numroutes;i++) {
entry =staticrouting->GetRoute(i);
std::cout << entry.GetDestNetwork() << "\t\t<< entry.GetDestNetworkMask() << "\t\t\t<< entry.GetGateway() <<
"\t\t\t<< entry.GetInterface() << \n"; }
return; }
void
ComputeResults (void)
{
double deltaT = (stop - warmup);
// Throughput Average by Flow (bps)
data.thrpAvgByFlow = (double) data.sumRxBytesByFlow * 8 / (data.nFlows * deltaT);
// Throughput Quadratic Average by Flow (bps2

data.thrpAvgQuadByFlow = (double) data.sumRxBytesQuadByFlow * 8*8 / (data.nFlows * deltaT*deltaT);
// Throughput Variance by Flow (bps2

data.thrpVarByFlow = data.thrpAvgQuadByFlow - data.thrpAvgByFlow * data.thrpAvgByFlow;
// Network Aggregated Throughput Average by Node (bps)
//data.netThrpAvgByNode = (double) data.sumRxBytesByFlow * 8 / (numFluxos * deltaT);
// Fairness Jain’s Index
data.fairness = (double) data.sumRxBytesByFlow * data.sumRxBytesByFlow / (data.nFlows * data.sumRxBytesQuadByFlow);
// Delay Mean by Packet (nanoseconds)
data.delayByPkt = (double) data.sumDelayFlow / data.sumRxPktsByFlow;
// Lost Ratio (%)
data.lostRatio = (double) 100 * data.sumLostPktsByFlow / data.sumTxPktsByFlow;
data.pdr = (double) 100 * data.sumRxPktsByFlow / data.sumTxPktsByFlow;
cout << "========================================"<< endl
<< "Simulation results:"<< endl
<< "Throughput Average by Flow (kbps):\t"<< data.thrpAvgByFlow / 1024.0 << endl
<< "Throughput Deviation by Flow (kbps):\t"<< sqrt (data.thrpVarByFlow) / 1024.0 << endl
// << "Network Aggregated Throughput Average by Node (kbps):\t<< data.netThrpAvgByNode / 1024.0 << endl
<< "Fairness Jain’s Index:\t"<< data.fairness << endl
<< "Delay Mean by Packet (seconds):\t"<< data.delayByPkt / 1e9 << endl
<< "Packet Lost Ratio (%):\t"<< data.lostRatio << endl
<< "Packet Delivery Ratio (%):\t"<< data.pdr<<endl<< endl<< endl;
cout << "Flows: << data.nFlows" << endl;
ofstream throughput;
throughput.open ("Topologia/Topologia1/v2/FlowMonitor/throughput-olsr/throughput-olsr-v2", ios::app);
throughput << "\t<< data.nFlows << "\t<< std::setprecision(4) << data.thrpAvgByFlow / 1024.0 << endl;
throughput.close ();
ofstream delay;
delay.open ("Topologia/Topologia1/v2/FlowMonitor/delay-olsr/delay-olsr-v2", ios::app);
delay << "\t<< data.nFlows << "\t<< std::setprecision(4) << data.delayByPkt / 1e6 << endl;
delay.close ();
ofstream fairness;
fairness.open ("Topologia/Topologia1/v2/FlowMonitor/fairness-olsr/fairness-olsr-v2", ios::app);
fairness << "\t<< data.nFlows << "\t<< std::setprecision(4) << data.fairness << endl;
fairness.close ();
ofstream pdr;
pdr.open ("Topologia/Topologia1/v2/FlowMonitor/PDR-olsr/pdr-olsr-v2", ios::app);
pdr << "\t<< data.nFlows << "\t<< std::setprecision(4) << data.pdr << endl;
pdr.close ();
ofstream packets;
packets.open ("Topologia/Topologia1/v2/FlowMonitor/packets-olsr/packets-olsr-v2", ios::app);
packets << "\t<< data.sumTxPktsByFlow << "\t<< std::setprecision (4) << data.sumRxPktsByFlow << endl;
packets.close (); 

}