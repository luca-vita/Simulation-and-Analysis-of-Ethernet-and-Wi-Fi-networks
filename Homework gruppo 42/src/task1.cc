#include "ns3/command-line.h"
#include "ns3/config.h"
#include "ns3/double.h"
#include "ns3/internet-stack-helper.h"
#include "ns3/ipv4-address-helper.h"
#include "ns3/log.h"
#include "ns3/mobility-helper.h"
#include "ns3/mobility-model.h"
#include "ns3/mobility-module.h"
#include "ns3/string.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/yans-wifi-channel.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/applications-module.h"
#include <iostream> 
#include <cstdlib>
#include <string>

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HW2_task1_Team_42");
 
int
main(int argc, char* argv[]) {

    bool verbose = false;
    bool useRtsCts = false;
    bool useNetAnim = false;

    CommandLine cmd(__FILE__);
    cmd.AddValue("verbose", "Abilita l'uso dei log per i server e i client", verbose);
    cmd.AddValue("useRtsCts", "Vuoi forzare l'utilizzo dell'handshake RTS/CTS?", useRtsCts);
    cmd.AddValue("useNetAnim", "Abilita NetAmin", useNetAnim);
    cmd.Parse(argc, argv);

    UintegerValue ctsThr = (useRtsCts ? UintegerValue(100) : UintegerValue(2200));
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", ctsThr);
        
    if (verbose){
        LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
        LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    NodeContainer wifiNodes;
    wifiNodes.Create(5);

    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper phy;
    phy.SetChannel(channel.Create());  

    //creazione strato mac in modalità adhoc
    WifiMacHelper wifiMac;
    wifiMac.SetType("ns3::AdhocWifiMac", "QosSupported", BooleanValue(false));

    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");

    NetDeviceContainer wifi_devices = wifi.Install(phy, wifiMac, wifiNodes);

    if(useRtsCts == true){
        //attivo pcap
        phy.EnablePcap("task1-on-2.pcap", wifi_devices.Get(2), true, true);
    }
    else{
        //attivo pcap
        phy.EnablePcap("task1-off-2.pcap", wifi_devices.Get(2), true, true);
    }  

    MobilityHelper mobility;
    mobility.SetPositionAllocator("ns3::GridPositionAllocator",
                                  "MinX",
                                  DoubleValue(0.0),
                                  "MinY",
                                  DoubleValue(0.0),
                                  "DeltaX",
                                  DoubleValue(5.0),
                                  "DeltaY",
                                  DoubleValue(10.0),
                                  "GridWidth",
                                  UintegerValue(3),
                                  "LayoutType",
                                  StringValue("RowFirst"));
    mobility.SetMobilityModel("ns3::RandomWalk2dMobilityModel",
                              "Bounds",
                              RectangleValue(Rectangle(-90, 90, -90, 90)));
    mobility.Install(wifiNodes);

    InternetStackHelper internet;
    internet.Install(wifiNodes);

    Ipv4AddressHelper address;
    address.SetBase("192.168.1.0", "255.255.255.0");
    Ipv4InterfaceContainer wifi_interfaces = address.Assign(wifi_devices);

    uint16_t port = 20;
    UdpEchoServerHelper UDPechoServer(port);
    
    //server è nodo 0
    ApplicationContainer apps = UDPechoServer.Install(wifiNodes.Get(0));
    apps.Start(Seconds(0));
    apps.Stop(Seconds(20));

    UdpEchoClientHelper UDPechoClientHelper_n4(wifi_interfaces.GetAddress(0), port);

    UDPechoClientHelper_n4.SetAttribute("MaxPackets", UintegerValue(2));
    UDPechoClientHelper_n4.SetAttribute("Interval", TimeValue(Seconds(1)));
    UDPechoClientHelper_n4.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer clientApps1 = UDPechoClientHelper_n4.Install(wifiNodes.Get(4)); //n4 client
    clientApps1.Start(Seconds(1.0));
    clientApps1.Stop(Seconds(4.0));

    UdpEchoClientHelper UDPechoClientHelper_n3(wifi_interfaces.GetAddress(0), port);//n3 cliient
    UDPechoClientHelper_n3.SetAttribute("MaxPackets", UintegerValue(2));
    UDPechoClientHelper_n3.SetAttribute("Interval", TimeValue(Seconds(2)));
    UDPechoClientHelper_n3.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer clientApps2 = UDPechoClientHelper_n3.Install(wifiNodes.Get(3));
    clientApps2.Start(Seconds(2.0));
    clientApps2.Stop(Seconds(5.0));

    if (useNetAnim == true) {
        if (useRtsCts == true) {
            AnimationInterface anim("wireless-task1-rts-on.xml");
            
            anim.EnablePacketMetadata();
            
            for (uint32_t i = 0; i < wifiNodes.GetN(); ++i) {
                if (i == 0) {
                    anim.UpdateNodeDescription(wifiNodes.Get(i), "SRV-0"); 
                    anim.UpdateNodeColor(wifiNodes.Get(i), 255, 0, 0);
                }   
                else if (i == 1) {
                    anim.UpdateNodeDescription(wifiNodes.Get(i), "HOC-1"); 
                    anim.UpdateNodeColor(wifiNodes.Get(i), 0, 0, 255);    
                }
                else if (i == 2) {
                    anim.UpdateNodeDescription(wifiNodes.Get(i), "HOC-2"); 
                    anim.UpdateNodeColor(wifiNodes.Get(i), 0, 0, 255);    
                }
                else if (i == 3) {
                    anim.UpdateNodeDescription(wifiNodes.Get(i), "CLI-3"); 
                    anim.UpdateNodeColor(wifiNodes.Get(i), 0, 255, 0);  
                }
                else if (i == 4) {
                    anim.UpdateNodeDescription(wifiNodes.Get(i), "CLI-4"); 
                    anim.UpdateNodeColor(wifiNodes.Get(i), 0, 255, 0);  
                }
            }
            anim.EnableWifiMacCounters( Seconds(0), Seconds(10) );
            anim.EnableWifiPhyCounters( Seconds(0), Seconds(10) );

            Simulator::Stop(Seconds(10));
            Simulator::Run();
            Simulator::Destroy();
        }
        if (useRtsCts == false) {
            AnimationInterface anim("wireless-task1-rts-off.xml"); 
           
            anim.EnablePacketMetadata();
           
            for (uint32_t i = 0; i < wifiNodes.GetN(); ++i) {
                if (i == 0) {
                    anim.UpdateNodeDescription(wifiNodes.Get(i), "SRV-0"); 
                    anim.UpdateNodeColor(wifiNodes.Get(i), 255, 0, 0);
                }   
                else if (i == 1) {
                    anim.UpdateNodeDescription(wifiNodes.Get(i), "HOC-1"); 
                    anim.UpdateNodeColor(wifiNodes.Get(i), 0, 0, 255);    
                }
                else if (i == 2) {
                    anim.UpdateNodeDescription(wifiNodes.Get(i), "HOC-2"); 
                    anim.UpdateNodeColor(wifiNodes.Get(i), 0, 0, 255);    
                }
                else if (i == 3) {
                    anim.UpdateNodeDescription(wifiNodes.Get(i), "CLI-3"); 
                    anim.UpdateNodeColor(wifiNodes.Get(i), 0, 255, 0);  
                }
                else if (i == 4) {
                    anim.UpdateNodeDescription(wifiNodes.Get(i), "CLI-4"); 
                    anim.UpdateNodeColor(wifiNodes.Get(i), 0, 255, 0);  
                }
            }
            anim.EnableWifiMacCounters( Seconds(0), Seconds(10) );
            anim.EnableWifiPhyCounters( Seconds(0), Seconds(10) );

            Simulator::Stop(Seconds(10));
            Simulator::Run();
            Simulator::Destroy();
        }
    }  
    else  {
        Simulator::Stop(Seconds(10));
        Simulator::Run();
        Simulator::Destroy(); 
    }

    NS_LOG_INFO("Run Simulation.");
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
