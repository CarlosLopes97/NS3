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
#include "ns3/applications-module.h"
#include "ns3/mobility-module.h"
#include "ns3/netanim-module.h"
using namespace ns3;

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
  nodes.Create (144);

  NodeContainer ab;
  ab.Add(server);
  ab.Add(nodes.Get(0));

  NodeContainer bc;
  bc.Add(nodes.Get(0));
  bc.Add(nodes.Get(1));

  NodeContainer all;
  
  all.Add(server);
  all.Add(nodes);

  MobilityHelper mobility; 
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0), //onde inicia no eixo X
                                 "MinY", DoubleValue (0.0), //onde inicia no eixo Y
                                 "DeltaX", DoubleValue (10.0), // Distância entre nós
                                 "DeltaY", DoubleValue (10.0), // Distância entre nós
                                 "GridWidth", UintegerValue (12), // Quantidade de colunas em uma linha
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


  PointToPointHelper pointToPoint;
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("2ms"));

  NetDeviceContainer devices;
  devices = pointToPoint.Install (ab);
  pointToPoint.SetDeviceAttribute ("DataRate", StringValue ("1Mbps"));
  pointToPoint.SetChannelAttribute ("Delay", StringValue ("5ms"));

  NetDeviceContainer devices2;
  devices2 = pointToPoint.Install (bc);


  InternetStackHelper stack;
  stack.Install (all);

  Ipv4AddressHelper address;
  address.SetBase ("10.1.1.0", "255.255.255.0");

  Ipv4InterfaceContainer p2pS = address.Assign (devices);

  address.SetBase ("10.1.2.0", "255.255.255.0");

  Ipv4InterfaceContainer p2pC = address.Assign (devices2);

  UdpEchoServerHelper echoServer (9);

  ApplicationContainer serverApps = echoServer.Install (server.Get (0));
  serverApps.Start (Seconds (1.0));
  serverApps.Stop (Seconds (10.0));

  UdpEchoClientHelper echoClient (p2pS.GetAddress (0), 9);
  echoClient.SetAttribute ("MaxPackets", UintegerValue (2));
  echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
  echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer clientApps = echoClient.Install (nodes.Get(1));
  clientApps.Start (Seconds (2.0));
  clientApps.Stop (Seconds (10.0));

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  pointToPoint.EnablePcapAll ("second", true);

//-------------------------Metodo-Animation-------------------------

      AnimationInterface anim ("first.xml"); // Mandatory
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

  Simulator::Stop (Seconds (25.0));
  Simulator::Run ();
  Simulator::Destroy ();
  return 0;
}
