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

const std::string lifx_broadcast_ip = "255.255.255.255";
const std::string control_group = "Living Room";
const std::string vlc_ip = "192.168.0.230";
const std::string vlc_auth = "OnZsY3Bhc3M=";

std::mutex manager_mutex;
int lightstate = 2;

using namespace lifx;
        
void discovery(Manager* manager, bool *bgthread)
{
	while(*bgthread)
	{
		manager_mutex.lock();
		manager->Discover();
		manager->PurgeOldDevices();
		//manager->ListGroups();
		manager_mutex.unlock();
		std::this_thread::sleep_for (std::chrono::seconds(5));
	}
}

void vlc_listener(VLC* vlc, Manager* manager)
{
    std::string playstate, fullscreen;
    bool success;
    
    while(1){
        success = vlc->GetState(playstate, fullscreen);
        if(success) {
            if(playstate == "playing" && fullscreen == "true" && lightstate != 1)
            {
                manager_mutex.lock();
                manager->LightsDown(control_group, true);
                manager_mutex.unlock();
                lightstate = 1;
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
    Manager * manager = new Manager(lifx_broadcast_ip);
    VLC* vlc = new VLC(vlc_ip, vlc_auth);
    
	bool bgthread = true;
    
	std::thread t1(discovery, manager, &bgthread);
    std::thread t2(vlc_listener, vlc, manager);
    
    // todo - handle termination
    
	t1.join();
	t2.join();
    
    return 0;
}
