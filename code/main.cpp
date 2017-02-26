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
//const std::string control_group = "Living Room";
//const std::string vlc_ip = "192.168.0.230";
const std::string vlc_auth = "OnZsY3Bhc3M=";

std::mutex manager_mutex;
int lightstate = 2;
volatile bool done = false;
Manager * manager;
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN   
#include <windows.h>
bool WINAPI signal_callback_handler(uint32_t dwType)
{
	manager->WriteDevices("C:/Users/Lans/Documents/Projects/lifx-api/LIFXCPPAPI/cache");
	
	done = true;

	exit(1);
}
#else
void signal_callback_handler(int signum)
{
    manager->WriteDevices("cache");
    done = true;
    
    exit(signum);
}
#endif   
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
	
	manager->Close();
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
                set = manager->LightsDown(manager->controlGroup, true);
                manager_mutex.unlock();
                if(set)
                {
                    lightstate = 1;
                }
            }
            else if((playstate != "playing" || fullscreen != "true")  && lightstate == 1)
            {
                manager_mutex.lock();
                manager->LightsRestore(manager->controlGroup);
                manager_mutex.unlock();
                lightstate = 2;
            }
        }

        std::this_thread::sleep_for (std::chrono::seconds(1));
    }

	vlc->Close();
}

int main(int argc, const char* argv[]) {

#ifdef WIN32
	if (!SetConsoleCtrlHandler((PHANDLER_ROUTINE)signal_callback_handler, TRUE)) {
		perror("Unable to install handler!\n");
		return EXIT_FAILURE;
	}
#else
    signal(SIGINT, signal_callback_handler);
#endif
	manager = new Manager(lifx_broadcast_ip);

	LIFXDeviceState state = LIFXDeviceState(0, 0, 65535, 5500, 65535, 0);

	manager->ReadDevices("cache");

	manager->ReadConfig();

	if (manager->lightsStartOn == 1)
	{
		for (auto it1 : manager->groups)
		{
			for (auto it : it1.second->devices)
			{
				manager->SetColorAndPower(it.second, false, false, &state, 0);
			}
		}
	}
    
    VLC* vlc = new VLC(manager->vlcIP, vlc_auth);
    
    std::thread t1(discovery, manager);
    std::thread t2(vlc_listener, vlc, manager);
	
    t1.join();
	t2.join();

    return 0;
}
