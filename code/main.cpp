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
#include "Base64Encoder.h"

using namespace lifx;

const std::string lifx_broadcast_ip = "255.255.255.255";

std::mutex manager_mutex;
volatile bool done = false;
Manager * manager;

#ifdef WIN32
#define WIN32_LEAN_AND_MEAN   
#include <windows.h>
bool WINAPI signal_callback_handler(uint32_t dwType)
{
	manager->WriteDevices("cache");
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
		if (manager->activeConfigNum != 0)
		{
			success = vlc->GetState(playstate, fullscreen);
			if (success) {
				if (playstate == "playing" && fullscreen == "true" && manager->lightState != LIGHTS_DOWN)
				{
					bool set = false;
					manager_mutex.lock();
					std::cout << "lights down\n";
					manager->LightsDown(manager->controlGroup, true);
					manager_mutex.unlock();
					
				}
				else if ((playstate != "playing" || fullscreen != "true") && manager->lightState == LIGHTS_DOWN)
				{
					manager_mutex.lock();
					std::cout << "lights restored\n";
					manager->LightsRestore(manager->controlGroup);
					manager_mutex.unlock();
					
				}
			}
		}
		else if (manager->lightState != LIGHTS_RESTORED)
		{
			manager_mutex.lock();
			std::cout << "lights restored 1\n";
			manager->LightsRestore(manager->controlGroup);
			manager_mutex.unlock();
		}

        std::this_thread::sleep_for (std::chrono::seconds(1));
    }

	vlc->Close();
}

#define VK_K 0x4b
#ifdef WIN32
void hotkey_listener(Manager* manager)
{
	BOOL ret;
	// register CTRL + 0 - CTRL + 9
	for (int i = 0x30; i < 0x40; i++)
	{
		ret = RegisterHotKey(NULL, i-0x30, MOD_CONTROL, i);
		if (!ret)
		{
			std::cerr << "Error registering hotkey CTRL + " << i << "\n";
		}
	}

	MSG   msg = { 0 };
	while (GetMessage(&msg, NULL, 0, 0) != 0 && !done) {
		if (msg.message == WM_HOTKEY) {
			if (msg.wParam < MAX_CONFIGS)
			{
				manager_mutex.lock();
				manager->activeConfigNum = msg.wParam;
				manager->lightState = LIGHTS_CONFIG_CHANGED;
				manager_mutex.unlock();
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}
#endif

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
	Base64Encoder b = Base64Encoder();
	std::string authstring = (manager->vlcUname + ":" + manager->vlcPass);
    
    VLC* vlc = new VLC(manager->vlcIP, b.encode(reinterpret_cast<const unsigned char*>(authstring.c_str()), authstring.length()));
    
    std::thread t1(discovery, manager);
    std::thread t2(vlc_listener, vlc, manager);
	std::thread t3(hotkey_listener, manager);
	
    t1.join();
	t2.join();
	t3.join();

    return 0;
}
