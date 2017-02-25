#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <sstream>
#include <memory>
#include <list>
#include <map>
#include <thread>
#include <fstream>
#include <cstring>
#include <mutex>
#include <chrono>
#include "Socket.h"
#include "Packet.h"
#include "Color.h"
#include "Manager.h"
#include "VLC.h"

using namespace lifx;
        
const std::string lifx_broadcast_ip = "255.255.255.255";
const std::string control_group = "Living Room";
const std::string vlc_ip = "192.168.0.230";
const std::string vlc_auth = "OnZsY3Bhc3M=";

std::mutex manager_mutex;
int lightstate = 2;
volatile bool done = false;
Manager * manager;

void signal_callback_handler(int signum)
{
    manager->WriteDevices("cache");
    done = true;
    
    exit(signum);
}
    
void discovery(Manager* manager)
{
	while(!done)
	{
		manager_mutex.lock();
		manager->Discover();
		//manager->ListGroups();
		manager_mutex.unlock();
		std::this_thread::sleep_for (std::chrono::seconds(5));
	}
}

void vlc_listener(VLC* vlc, Manager* manager)
{
    std::string playstate, fullscreen;
    bool success;
    
    while(!done){
        success = vlc->GetState(playstate, fullscreen);
        if(success) {
            if(playstate == "playing" && fullscreen == "true" && lightstate != 1)
            {
                bool set = false;
                manager_mutex.lock();
                set = manager->LightsDown(control_group, true);
                manager_mutex.unlock();
                if(set)
                {
                    lightstate = 1;
                }
            }
            else if((playstate != "playing" || fullscreen != "true")  && lightstate == 1)
            {
                manager_mutex.lock();
                manager->LightsRestore(control_group);
                manager_mutex.unlock();
                lightstate = 2;
            }
        }
        std::this_thread::sleep_for (std::chrono::seconds(1));
    }
}

int main(int argc, const char* argv[]) {
    std::shared_ptr<Socket> http_socket;
    signal(SIGINT, signal_callback_handler);
	 
    manager = new Manager(lifx_broadcast_ip);
    VLC* vlc = new VLC(vlc_ip, vlc_auth);
    LIFXDeviceState state = LIFXDeviceState(0, 0, 65535, 5500, 65535, 0);
    
    
    manager->ReadDevices("cache");
    for(auto it : manager->groups.begin()->second->devices)
    {
        manager->SetColorAndPower(it.second, false, false, &state, 0);
    }
    manager->ReadConfig();
    std::thread t1(discovery, manager);
    std::thread t2(vlc_listener, vlc, manager);
	
    t1.join();
	t2.join();
    
    return 0;
}
