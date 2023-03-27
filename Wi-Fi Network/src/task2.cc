
#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/network-module.h"
#include "ns3/ssid.h"
#include "ns3/yans-wifi-helper.h"
#include "ns3/animation-interface.h"
#include "ns3/rng-seed-manager.h"
#include <unistd.h>
#include <string>
#include <iostream>

/*nodi presenti in topologia n0 n1 n2 n3 n4 n5(è ap) 
- n0 server
- n3 e n4, sono client
- n5 è AP
- n1 n2 n5 sono base station normali
*/

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("HW2_Task2_Team_42");

int main(int argc, char* argv[]){

    bool useRtsCts = false ;
    bool verbose = false;
    bool useNetAnim = false;
    std::string ssid_name = "TLC2022";

    CommandLine cmd(__FILE__);
    cmd.AddValue("useRtsCts", "Vuoi forzare l'utilizzo dell'handshake RTS/CTS?", useRtsCts);
    cmd.AddValue("verbose", "Abilita l'uso dei log per server e client", verbose);
    cmd.AddValue("useNetAnim", "Abilita NetAnim", useNetAnim);
    cmd.AddValue("ssid", "Inserisci ssid", ssid_name);
    
    cmd.Parse(argc, argv);
    
    UintegerValue ctsThr = (useRtsCts ? UintegerValue(100) : UintegerValue(2200) );
    Config::SetDefault("ns3::WifiRemoteStationManager::RtsCtsThreshold", ctsThr);

    if(verbose == true){
       LogComponentEnable("UdpEchoClientApplication", LOG_LEVEL_INFO);
       LogComponentEnable("UdpEchoServerApplication", LOG_LEVEL_INFO);
    }

    NodeContainer wifiNodes;
    wifiNodes.Create(5);//ne creo 5 di cui uno è ap
    NodeContainer wifiApNode ;
    wifiApNode.Create(1); //nodo 1 di wifiApnode* è AP

    /*PHYSICAL LAYER*/
    YansWifiChannelHelper channel = YansWifiChannelHelper::Default();
    YansWifiPhyHelper physic;
    physic.SetChannel(channel.Create());
    
    WifiMacHelper mac;
    Ssid ssid = Ssid(ssid_name);

//  LINK LAYER
    WifiHelper wifi;
    wifi.SetStandard(WIFI_STANDARD_80211g);
    wifi.SetRemoteStationManager("ns3::AarfWifiManager");
    
    //device contaiener per Ap
    NetDeviceContainer apDevices;
    mac.SetType("ns3::ApWifiMac", "Ssid", SsidValue(ssid), "QosSupported", BooleanValue(false));
    apDevices = wifi.Install(physic, mac, wifiApNode);
    
   //device container per nodi collegati ad ap
    NetDeviceContainer staDevices;
    mac.SetType("ns3::StaWifiMac", "Ssid", SsidValue(ssid), "ActiveProbing", BooleanValue(false), "QosSupported", BooleanValue(false));
    staDevices = wifi.Install(physic, mac, wifiNodes);    

    if(useRtsCts == true){
        //attivo pcap
        physic.EnablePcap("task2-on-0.pcap", apDevices.Get(0), true, true);
        physic.EnablePcap("task2-on-4.pcap", staDevices.Get(4),  true, true);
    }
    else{
        //attivo pcap
        physic.EnablePcap("task2-off-0.pcap", apDevices.Get(0), true, true);
        physic.EnablePcap("task2-off-4.pcap", staDevices.Get(4),  true, true);
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

    //AP non si muove
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(wifiApNode);

    InternetStackHelper stack;//installo i device container 
    stack.Install(wifiApNode);
    stack.Install(wifiNodes);

//LIVELLO TRANSPORTO E NETWORK
    Ipv4AddressHelper address;
    address.SetBase("192.168.1.0", "255.255.255.0"); 
    Ipv4InterfaceContainer StaInterfaces;
    Ipv4InterfaceContainer ApInterface;

    StaInterfaces = address.Assign(staDevices);
    ApInterface =  address.Assign(apDevices);
    

//LIVELLO APPLICAZIONE
    uint16_t port = 21;
    UdpEchoServerHelper UdpEchoServer(port);

//  server è nodo 0
    ApplicationContainer apps = UdpEchoServer.Install(wifiNodes.Get(0)); //nodo 0 riceve
    apps.Start(Seconds(0));
    apps.Stop(Seconds(15));//server rimane attivo fino a fine simulazione 


    UdpEchoClientHelper UDPechoClientHelper_n3(StaInterfaces.GetAddress(0), port); //destinatario è server nodo 0
    UDPechoClientHelper_n3.SetAttribute("MaxPackets", UintegerValue(2));
    UDPechoClientHelper_n3.SetAttribute("Interval", TimeValue(Seconds(2)));// da 2 a 4
    UDPechoClientHelper_n3.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer clientApps_n3 = UDPechoClientHelper_n3.Install(wifiNodes.Get(3));//installo sul mittente cioe nodo 3
    clientApps_n3.Start(Seconds(2));
    clientApps_n3.Stop(Seconds(6));

    UdpEchoClientHelper UDPechoClientHelper_n4(StaInterfaces.GetAddress(0), port); //destinatario è server nodo 0
    UDPechoClientHelper_n4.SetAttribute("MaxPackets", UintegerValue(2));
    UDPechoClientHelper_n4.SetAttribute("Interval", TimeValue(Seconds(3))); // da 1 a 4
    UDPechoClientHelper_n4.SetAttribute("PacketSize", UintegerValue(512));

    ApplicationContainer clientApps_n4 = UDPechoClientHelper_n4.Install(wifiNodes.Get(4));//installo sul mittente cioe nodo 4
    clientApps_n4.Start(Seconds(1));
    clientApps_n4.Stop(Seconds(6));


    Ipv4GlobalRoutingHelper::PopulateRoutingTables();
    Simulator::Stop(Seconds(10.0));

        //----NETANIM----       
    if(useNetAnim == true){//se è attivato net anim
    //cambia il nome di file xlm in base a se rts è attivo o meno 
        
        if(useRtsCts == true){

            AnimationInterface anim("wireless-task2-rts-on.xlm");
            
            anim.EnablePacketMetadata();
            anim.UpdateNodeDescription(wifiNodes.Get(0),"SRV-0");//nodo server n0
            anim.UpdateNodeColor(wifiNodes.Get(0), 255 , 0 , 0);//colore rosso

            anim.UpdateNodeDescription(wifiNodes.Get(3),"CLI-3");//nodo n3 client
            anim.UpdateNodeColor(wifiNodes.Get(3), 0 , 255 , 0);//colore verde

            anim.UpdateNodeDescription(wifiNodes.Get(4),"CLI-4");//nodo n4 client
            anim.UpdateNodeColor(wifiNodes.Get(4), 0 , 255 , 0);//colore verde

            anim.UpdateNodeDescription(wifiApNode.Get(0),"AP");//nodo ap è nodo 6
            anim.UpdateNodeColor(wifiApNode.Get(0), 66 , 49 , 137);//colore viola

            anim.UpdateNodeDescription(wifiNodes.Get(1),"STA-1");//nodo n1 NORMALE
            anim.UpdateNodeColor(wifiNodes.Get(1), 0 , 0 , 255);//colore blu

            anim.UpdateNodeDescription(wifiNodes.Get(2),"STA-2");//nodo n2 NORMALE
            anim.UpdateNodeColor(wifiNodes.Get(2), 0 , 0 , 255);//colore blu
            
            anim.EnableWifiMacCounters( Seconds(0), Seconds(10) );
            anim.EnableWifiPhyCounters( Seconds(0), Seconds(10) );

            Simulator::Stop(Seconds(10));
            Simulator::Run();
            Simulator::Destroy();

        }
        else{
            AnimationInterface anim("wireless-task2-rts-off.xlm");

            anim.EnablePacketMetadata();

            anim.UpdateNodeDescription(wifiNodes.Get(0),"SRV-0");//nodo server n0
            anim.UpdateNodeColor(wifiNodes.Get(0), 255 , 0 , 0);//colore rosso

            anim.UpdateNodeDescription(wifiNodes.Get(3),"CLI-3");//nodo n3 client
            anim.UpdateNodeColor(wifiNodes.Get(3), 0 , 255 , 0);//colore verde

            anim.UpdateNodeDescription(wifiNodes.Get(4),"CLI-4");//nodo n4 client
            anim.UpdateNodeColor(wifiNodes.Get(4), 0 , 255 , 0);//colore verde

            anim.UpdateNodeDescription(wifiApNode.Get(0),"AP");//nodo ap è nodo 6
            anim.UpdateNodeColor(wifiApNode.Get(0), 66 , 49 , 137);//colore viola

            anim.UpdateNodeDescription(wifiNodes.Get(1),"STA-1");//nodo n1 NORMALE
            anim.UpdateNodeColor(wifiNodes.Get(1), 0 , 0 , 255);//colore blu

            anim.UpdateNodeDescription(wifiNodes.Get(2),"STA-2");//nodo n2 NORMALE
            anim.UpdateNodeColor(wifiNodes.Get(2), 0 , 0 , 255);//colore blu
            
            anim.EnableWifiMacCounters( Seconds(0), Seconds(10) );
            anim.EnableWifiPhyCounters( Seconds(0), Seconds(10) );

            Simulator::Stop(Seconds(10));
            Simulator::Run();
            Simulator::Destroy();
        }  
              
    }
    else{        
        Simulator::Stop(Seconds(10));
        Simulator::Run();
        Simulator::Destroy();
    }
    
    NS_LOG_INFO("Run Simulation.");
    Simulator::Run();
    Simulator::Destroy();
    return 0;

}