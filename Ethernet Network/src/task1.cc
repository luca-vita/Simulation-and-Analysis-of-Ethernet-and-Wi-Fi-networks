#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include <iostream> 
#include <cstdlib>
#include <cstring>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("Task_1_Team_42");

int main(int argc, char* argv[]){

    Time::SetResolution(Time::NS);
    LogComponentEnable("OnOffApplication", LOG_LEVEL_INFO);
    LogComponentEnable("PacketSink", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);

    CommandLine cmd;
    int configuration;
    cmd.AddValue("configuration", "Number of configuration to execute", configuration);
    cmd.Parse(argc, argv);

    int numeroNodi = 9;
    NodeContainer c;
    c.Create(numeroNodi);

    // p2p
    NodeContainer n1_n3 = NodeContainer(c.Get(1), c.Get(3));
    NodeContainer n3_n6 = NodeContainer(c.Get(3), c.Get(6));
    NodeContainer n6_n5 = NodeContainer(c.Get(6), c.Get(5));
    NodeContainer n5_n4 = NodeContainer(c.Get(5), c.Get(4));

    // lan sx
    NodeContainer csmaNodes_sx = NodeContainer(c.Get(0), c.Get(1), c.Get(2));
    // lan dx
    NodeContainer csmaNodes_dx = NodeContainer(c.Get(6), c.Get(7), c.Get(8));

    // We create the channels first without any IP addressing information
    // First make and configure the helper, so that it will put the appropriate
    // attributes on the network interfaces and channels we are about to install.
    PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("80Mbps"));
    pointToPoint.SetChannelAttribute("Delay", TimeValue(MicroSeconds(5)));
    
    // install devices and channels connecting our topology.
    NetDeviceContainer n1_n3Devices = pointToPoint.Install(n1_n3);
    NetDeviceContainer n3_n6Devices = pointToPoint.Install(n3_n6);
    NetDeviceContainer n6_n5Devices = pointToPoint.Install(n6_n5);
    NetDeviceContainer n5_n4Devices = pointToPoint.Install(n5_n4);

    // assegno alle varie sezione i valori di data rate e delay quindi i valori del canale per LAN sx
    CsmaHelper csmaHelper_sx;
    csmaHelper_sx.SetChannelAttribute("DataRate", StringValue("25Mbps"));
    csmaHelper_sx.SetChannelAttribute("Delay", TimeValue(MicroSeconds(10)));
    // creo device per inserire nei nodi le interfacce di rete
    NetDeviceContainer csmaDevices_sx = csmaHelper_sx.Install(csmaNodes_sx);

    // assegno alle varie sezione i valori di data rate e delay quindi i valori del canale per LAN dx
    CsmaHelper csmaHelper_dx;
    csmaHelper_dx.SetChannelAttribute("DataRate", StringValue("30Mbps"));
    csmaHelper_dx.SetChannelAttribute("Delay", TimeValue(MicroSeconds(20)));
    // creo device per inserire nei nodi le interfacce di rete
    NetDeviceContainer csmaDevices_dx = csmaHelper_dx.Install(csmaNodes_dx);

    InternetStackHelper stack;
    stack.Install(csmaNodes_sx);
    stack.Install(csmaNodes_dx);
    stack.Install(c.Get(3));
    stack.Install(c.Get(4));
    stack.Install(c.Get(5));

    // assegnazione indirizzi ip
    Ipv4AddressHelper address;
    address.SetBase("10.0.1.0", "255.255.255.252");
    Ipv4InterfaceContainer n1_n3_Interfaces;
    n1_n3_Interfaces = address.Assign(n1_n3Devices);

    address.SetBase("10.0.2.0", "255.255.255.252");
    Ipv4InterfaceContainer n3_n6_Interfaces;
    n3_n6_Interfaces = address.Assign(n3_n6Devices);

    address.SetBase("10.0.4.0", "255.255.255.252");
    Ipv4InterfaceContainer n6_n5_Interfaces;
    n6_n5_Interfaces = address.Assign(n6_n5Devices);

    address.SetBase("10.0.3.0", "255.255.255.252");
    Ipv4InterfaceContainer n5_n4_Interfaces;
    n5_n4_Interfaces = address.Assign(n5_n4Devices);

    address.SetBase("192.138.1.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces_sx;
    csmaInterfaces_sx = address.Assign(csmaDevices_sx);

    address.SetBase("192.138.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csmaInterfaces_dx;
    csmaInterfaces_dx = address.Assign(csmaDevices_dx);

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    
    //CONFIGURAZIONE 0
    if (configuration == 0){ // server n2 e client n4 

        pointToPoint.EnablePcap("task1-0-n3.pcap", n1_n3Devices.Get(1), true,true);
        pointToPoint.EnablePcap("task1-0-n6.pcap", n3_n6Devices.Get(1), true,true);
        csmaHelper_dx.EnablePcap("task1-0-n6csma.pcap", csmaDevices_dx.Get(0), true,true);
        pointToPoint.EnablePcap("task1-0-n5.pcap", n5_n4Devices.Get(0), true,true);
    
        AsciiTraceHelper ascii;
        csmaHelper_sx.EnableAscii(ascii.CreateFileStream("task1-0-n2.tr"), csmaDevices_sx.Get(2));
        pointToPoint.EnableAscii(ascii.CreateFileStream("task1-0-n4.tr"), n5_n4Devices.Get(1));
        
        // Create a packet sink to receive these packets on n2...
        uint16_t port_n2 = 2400;
        // pongo packet sink su nodo
        PacketSinkHelper sink("ns3::TcpSocketFactory",Address(InetSocketAddress(csmaInterfaces_sx.GetAddress(2), port_n2))); // secondo parametro è address del sink
        ApplicationContainer app_sink = sink.Install(csmaNodes_sx.Get(2)); 
        app_sink.Start(Seconds(0.0));
        app_sink.Stop(Seconds(20.0)); 

        // Create a packet sink to receive these packets
        OnOffHelper clientHelper("ns3::TcpSocketFactory",Address(InetSocketAddress( csmaInterfaces_sx.GetAddress(2), port_n2))); // secondo parametro è address del nood a cui vuoi inviare roba
        clientHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]"));
        clientHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        clientHelper.SetAttribute("PacketSize", UintegerValue(1500));
        
        ApplicationContainer app_onoff = clientHelper.Install(n5_n4.Get(1)); 
        app_onoff.Start(Seconds(3.0));
        app_onoff.Stop(Seconds(15.0));
        
    }
    
    //CONFIGURAZIONE 1
    if (configuration == 1){ // server client n4 e n8 // client n0 e n2

        pointToPoint.EnablePcap("task1-1-n3.pcap", n1_n3Devices.Get(1), true,true);
        pointToPoint.EnablePcap("task1-1-n6.pcap", n3_n6Devices.Get(1), true,true);
        pointToPoint.EnablePcap("task1-1-n5.pcap", n5_n4Devices.Get(0), true,true); 
        csmaHelper_dx.EnablePcap("task1-1-n6csma.pcap", csmaDevices_dx.Get(0), true,true);


        AsciiTraceHelper ascii;//vanno solo su client e server 
        pointToPoint.EnableAscii(ascii.CreateFileStream("task1-1-n4.tr"), n5_n4Devices.Get(1) );//n4 server
        csmaHelper_dx.EnableAscii(ascii.CreateFileStream("task1-1-n8.tr"), csmaDevices_dx.Get(2) ); //server n8
        csmaHelper_sx.EnableAscii(ascii.CreateFileStream("task1-1-n0.tr"), csmaDevices_sx.Get(0) ); //client n0
        csmaHelper_sx.EnableAscii(ascii.CreateFileStream("task1-1-n2.tr"), csmaDevices_sx.Get(2) ); //client n2

        // Create a packet sink to receive these packets on n2 and n0...
        uint16_t port_n2 = 2400;
        uint16_t port_n0 = 7777;
        // pongo packet sink su nodo n2
        PacketSinkHelper sink_n2("ns3::TcpSocketFactory", Address( InetSocketAddress(csmaInterfaces_sx.GetAddress(2), port_n2) ) );
        // installo applicazione sink sul nodo2
        ApplicationContainer app_sink_n2 = sink_n2.Install(csmaNodes_sx.Get(2)); 
        app_sink_n2.Start(Seconds(0.0));
        app_sink_n2.Stop(Seconds(20.0)); 

        // pongo packet sink su nodo n0
        PacketSinkHelper sink_n0("ns3::TcpSocketFactory", Address( InetSocketAddress(csmaInterfaces_sx.GetAddress(0), port_n0)) );
        // installo applicazione sink sul nodo2
        ApplicationContainer app_sink_n0 = sink_n0.Install(csmaNodes_sx.Get(0)); 
        app_sink_n0.Start(Seconds(0.0));
        app_sink_n0.Stop(Seconds(20.0));

        OnOffHelper clientHelper_n4("ns3::TcpSocketFactory", Address(InetSocketAddress( csmaInterfaces_sx.GetAddress(0),port_n0))); 
        clientHelper_n4.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]")); 
        clientHelper_n4.SetAttribute("OffTime",StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        clientHelper_n4.SetAttribute("PacketSize", UintegerValue(2500));

        ApplicationContainer app_onoff_n4 = clientHelper_n4.Install(n5_n4.Get(1)); // onoff LO VOGLIO installato su nodo n4
        app_onoff_n4.Start(Seconds(5.0));
        app_onoff_n4.Stop(Seconds(15.0));

        OnOffHelper clientHelper_n8("ns3::TcpSocketFactory", Address(InetSocketAddress( csmaInterfaces_sx.GetAddress(2),port_n2))); 
        clientHelper_n8.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]")); 
        clientHelper_n8.SetAttribute("OffTime",StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        clientHelper_n8.SetAttribute("PacketSize", UintegerValue(4500));

        ApplicationContainer app_onoff_n8 = clientHelper_n8.Install(csmaNodes_dx.Get(2)); // onoff LO VOGLIO installato su nodo n8
        app_onoff_n8.Start(Seconds(2.0));
        app_onoff_n8.Stop(Seconds(9.0));
    }
    
    //CONFIGURAZIONE 2
    if (configuration == 2) { 

        pointToPoint.EnablePcap("task1-2-n3.pcap", n1_n3Devices.Get(1), true, true);
        pointToPoint.EnablePcap("task1-2-n6.pcap", n6_n5Devices.Get(0), true, true); 
        pointToPoint.EnablePcap("task1-2-n5.pcap", n5_n4Devices.Get(0), true,true);
        csmaHelper_dx.EnablePcap("task1-2-n6csma.pcap", csmaDevices_dx.Get(0), true,true);

        //abilito ascii tracing solo su client e server 
        AsciiTraceHelper ascii;
        csmaHelper_sx.EnableAscii(ascii.CreateFileStream("task1-2-n2.tr"), csmaDevices_sx.Get(2) );//n2 server 
        csmaHelper_dx.EnableAscii(ascii.CreateFileStream("task1-2-n8.tr"), csmaDevices_dx.Get(2) ); //client n8
        pointToPoint.EnableAscii(ascii.CreateFileStream("task1-2-n4.tr"), n5_n4Devices.Get(1) ); //client n4
        csmaHelper_dx.EnableAscii(ascii.CreateFileStream("task1-2-n7.tr"), csmaDevices_dx.Get(1) ); //client n7
        csmaHelper_sx.EnableAscii(ascii.CreateFileStream("task1-2-n0.tr"), csmaDevices_sx.Get(0) );//n0 server

        //pongo udp echo server su n2 alla porta 63
        uint16_t port = 63;
        UdpEchoServerHelper UDPechoServer(port);
        ApplicationContainer apps = UDPechoServer.Install(c.Get(2)); 
        apps.Start(Seconds(0));
        apps.Stop(Seconds(20));

        //UdpEchoClient N 8                     //lo stiamo inviando a nodo 2
        UdpEchoClientHelper UDPechoClientHelper(csmaInterfaces_sx.GetAddress(2), port);
        UDPechoClientHelper.SetAttribute("MaxPackets", UintegerValue(5));
        UDPechoClientHelper.SetAttribute("Interval", TimeValue(Seconds(2)));
        UDPechoClientHelper.SetAttribute("PacketSize", UintegerValue(2560));
        
        ApplicationContainer clientApps1 = UDPechoClientHelper.Install(csmaNodes_dx.Get(2)); //installato su nodo 8
        
        char* fill[8];
        memcpy(fill, "5841521", 8); // somma delle nostre matricole
        uint8_t* payload = (uint8_t*)&fill;
        UDPechoClientHelper.SetFill( clientApps1.Get(0) ,  payload, 7 ,2560); 
        clientApps1.Start(Seconds(3.0));                          
        clientApps1.Stop(Seconds(15));

        //installo il tcp sink sul nodo2
        uint16_t  port_n2 = 2600;
        PacketSinkHelper sink_n2("ns3::TcpSocketFactory", Address( InetSocketAddress(csmaInterfaces_sx.GetAddress(2), port_n2) ) );
        ApplicationContainer app_sink_n2 = sink_n2.Install(csmaNodes_sx.Get(2)); 
        app_sink_n2.Start(Seconds(0.0));
        app_sink_n2.Stop(Seconds(20.0));        

        //pongo udp sink sul nodo n0 porta 2500
        uint16_t port_n0 = 2500;
        PacketSinkHelper sink_n0("ns3::UdpSocketFactory", Address(InetSocketAddress(csmaInterfaces_sx.GetAddress(0), port_n0)));
        ApplicationContainer app_sink_n0 = sink_n0.Install(csmaNodes_sx.Get(0)); 
        app_sink_n0.Start(Seconds(0));
        app_sink_n0.Stop(Seconds(20.0));  
        
        //pongo tcp onoff client su n4 che invia a n2
        OnOffHelper TCPclientHelper("ns3::TcpSocketFactory",Address(InetSocketAddress(csmaInterfaces_sx.GetAddress(2), port_n2 ))); 
        TCPclientHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]")); 
        TCPclientHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        TCPclientHelper.SetAttribute("PacketSize", UintegerValue(3000));
        
        ApplicationContainer app_TCPonoff = TCPclientHelper.Install(n5_n4.Get(1)); 
        app_TCPonoff.Start(Seconds(3.0));
        app_TCPonoff.Stop(Seconds(9.0));

        //pongo udp onoff client su n7 che invia a n0
        OnOffHelper UDPclientHelper("ns3::UdpSocketFactory",Address(InetSocketAddress(csmaInterfaces_sx.GetAddress(0), port_n0 )));
        UDPclientHelper.SetAttribute("OnTime", StringValue("ns3::ConstantRandomVariable[Constant=1]")); 
        UDPclientHelper.SetAttribute("OffTime", StringValue("ns3::ConstantRandomVariable[Constant=0]"));
        UDPclientHelper.SetAttribute("PacketSize", UintegerValue(3000));
        
        ApplicationContainer app_UDPonoff = UDPclientHelper.Install(csmaNodes_dx.Get(1)); 
        app_UDPonoff.Start(Seconds(5.0));
        app_UDPonoff.Stop(Seconds(15.0));
    }
   
    Simulator::Run();     
    Simulator::Stop(Seconds(20.0));  // FA PARTIRE LA SIMULAZIONE DEVE TERMINARE IN 20 SECONDI
    Simulator::Destroy();   // FA CLEAN UP DELLE RISORSE E TERMINA
    return 0;

}
