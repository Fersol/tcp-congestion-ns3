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
 
 // Network topology
 //
 //       n0 ----------- n1
 //            500 Kbps
 //             5 ms
 //
 // - Flow from n0 to n1 using BulkSendApplication.
 // - Tracing of queues and packet receptions to file "tcp-bulk-send.tr"
 //   and pcap tracing available when tracing is turned on.
 
 #include <string>
 #include <fstream>
 #include "ns3/core-module.h"
 #include "ns3/point-to-point-module.h"
 #include "ns3/internet-module.h"
 #include "ns3/applications-module.h"
 #include "ns3/network-module.h"
 #include "ns3/packet-sink.h"
 #include "ns3/object.h"
 #include "ns3/traced-value.h"
  #include "ns3/tcp-header.h"
   #include "ns3/udp-header.h"
   #include "ns3/enum.h"
   #include "ns3/event-id.h"
   #include "ns3/flow-monitor-helper.h"
   #include "ns3/ipv4-global-routing-helper.h"
 #include "ns3/trace-source-accessor.h"
#include "ns3/traffic-control-helper.h"
 #include <iostream>
 
 using namespace ns3;
 
 NS_LOG_COMPONENT_DEFINE ("TcpBulkSendExample");

 static Ptr<OutputStreamWrapper> cWndStream;
 static uint32_t cWndValue;
 static bool firstCwnd = true;
 
static void 
 CwndTracer (uint32_t oldval, uint32_t newval)
 {
   //std::cout << Simulator::Now().GetSeconds() <<" Moving cwnd from " << oldval << " to " << newval << '\n';
   if (firstCwnd)
     {
       *cWndStream->GetStream () << "0.0 " << oldval << std::endl;
       firstCwnd = false;
     }
   *cWndStream->GetStream () << Simulator::Now ().GetSeconds () << " " << newval << std::endl;
   cWndValue = newval;
 }

static void
TraceCwnd(std::string name){
   std::cout << name << "\n";
   AsciiTraceHelper ascii;
   cWndStream = ascii.CreateFileStream (name.c_str ());
   Config::ConnectWithoutContext ("/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&CwndTracer));
}

 int
 main (int argc, char *argv[])
 {
 
   bool tracing = false;
   uint32_t maxBytes = 0;
 
 //
 // Allow the user to override any of the defaults at
 // run-time, via command-line arguments
 //
   CommandLine cmd;
   cmd.AddValue ("tracing", "Flag to enable/disable tracing", tracing);
   cmd.AddValue ("maxBytes",
                 "Total number of bytes for application to send", maxBytes);
   cmd.Parse (argc, argv);

   //Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpNewReno"));
   Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpVegas"));
   //Config::SetDefault ("ns3::TcpL4Protocol::SocketType", StringValue ("ns3::TcpBic"));
   //Config::SetDefault("ns3::TcpSocketBase::MaxWindowSize", UintegerValue (65536));
   Config::SetDefault("ns3::TcpSocketBase::Sack", BooleanValue (false));
   Config::SetDefault("ns3::TcpSocketBase::Timestamp", BooleanValue (false));
   Config::SetDefault("ns3::TcpSocket::SegmentSize", UintegerValue (1460));
   Config::SetDefault("ns3::TcpSocket::SndBufSize", UintegerValue (13107200));
   Config::SetDefault("ns3::TcpSocket::RcvBufSize", UintegerValue (13107200));
   Config::SetDefault("ns3::TcpSocket::InitialCwnd", UintegerValue (1));
   Config::SetDefault("ns3::TcpSocket::InitialSlowStartThreshold", UintegerValue (1));
   
 //
 // Explicitly create the nodes required by the topology (shown above).
 //
   NS_LOG_INFO ("Create nodes.");
   NodeContainer gateway;
   gateway.Create (1);
   NodeContainer sources;
   sources.Create (1);
   NodeContainer sinks;
   sinks.Create (1);

   TrafficControlHelper tcHelper;
 
 
   NS_LOG_INFO ("Create channels.");
 
 //
 // Explicitly create the point-to-point link required by the topology (shown above).
 //
   PointToPointHelper sourceToGateway;
   sourceToGateway.SetDeviceAttribute ("DataRate", StringValue ("100Mbps"));
   sourceToGateway.SetChannelAttribute ("Delay", StringValue ("10ms"));
   sourceToGateway.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("50p"));
   sourceToGateway.SetDeviceAttribute("InterframeGap", StringValue("0ms"));
   
   PointToPointHelper gatewayToSink;
   gatewayToSink.SetDeviceAttribute ("DataRate", StringValue ("50Mbps"));
   gatewayToSink.SetChannelAttribute ("Delay", StringValue ("10ms"));
   gatewayToSink.SetQueue ("ns3::DropTailQueue", "MaxSize", StringValue ("50p"));
   sourceToGateway.SetDeviceAttribute("InterframeGap", StringValue("0ms"));
 
 //
 // Install the internet stack on the nodes
 //
   InternetStackHelper internet;
   internet.InstallAll ();
 
 //
 // We've got the "hardware" in place.  Now we need to add IP addresses.
 //
   NS_LOG_INFO ("Assign IP Addresses.");
   Ipv4AddressHelper ipv4;
   ipv4.SetBase ("10.1.1.0", "255.255.255.0");

   Ipv4InterfaceContainer sink_interfaces;
   
   NetDeviceContainer devs;
   devs =  sourceToGateway.Install(sources.Get(0), gateway.Get(0));
   //std::cout << sourceToGateway.GetDeviceAttribute("InterframeGap").GetSeconds() << "\n";
   
   ipv4.NewNetwork();
   Ipv4InterfaceContainer interfaces = ipv4.Assign(devs);
   devs = gatewayToSink.Install(gateway.Get(0), sinks.Get(0));
   
   ipv4.NewNetwork();
   interfaces = ipv4.Assign(devs);
   sink_interfaces.Add(interfaces.Get(1));
        
   Ipv4GlobalRoutingHelper::PopulateRoutingTables ();
   
   NS_LOG_INFO ("Create Applications.");
 
 //
 // Create a BulkSendApplication and install it on node 0
 //
   uint16_t port = 9;  // well-known echo port number
 
   Address sinkLocalAddress (InetSocketAddress(Ipv4Address::GetAny(), port));  

   PacketSinkHelper sinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
   //sinkHelper.SetAttribute("Protocol", TypeIdValue(TcpSocketFactory::GetTypeId ())); 
   ApplicationContainer sinkApps = sinkHelper.Install (sinks);
   sinkApps.Start (Seconds (0.0));
   sinkApps.Stop (Seconds (120.0));



   AddressValue remoteAddress (InetSocketAddress(sink_interfaces.GetAddress(0,0), port));
   BulkSendHelper source ("ns3::TcpSocketFactory", Address());
   source.SetAttribute("Remote", remoteAddress);
   // Set the amount of data to send in bytes.  Zero is unlimited.
   source.SetAttribute ("MaxBytes", UintegerValue (maxBytes));
   ApplicationContainer sourceApp = source.Install (sources.Get (0));
   sourceApp.Start (Seconds (0.0));
   sourceApp.Stop (Seconds (120.0));
        
   Config::ConnectWithoutContext ("/NodeList/1/$ns3::TcpL4Protocol/SocketList/0/CongestionWindow", MakeCallback (&CwndTracer));
   
   Simulator::Schedule(Seconds(0.001), &TraceCwnd, "cwnd.tr");

 // Set up tracing if enabled
 //
 //  if (tracing)
  //   {
   //    AsciiTraceHelper ascii;
   //    pointToPoint.EnableAsciiAll (ascii.CreateFileStream ("tcp-bulk-send.tr"));
   //    pointToPoint.EnablePcapAll ("tcp-bulk-send", false);
  //   }
 
 //
 // Now, do the actual simulation.
 //
   NS_LOG_INFO ("Run Simulation.");
   Simulator::Stop (Seconds (120.0));
   Simulator::Run ();

   Simulator::Destroy ();
   NS_LOG_INFO ("Done.");
 
   Ptr<PacketSink> sink1 = DynamicCast<PacketSink> (sinkApps.Get (0));
   std::cout << "Total Bytes Received: " << sink1->GetTotalRx () << std::endl;
 }
