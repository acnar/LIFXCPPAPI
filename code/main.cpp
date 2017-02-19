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

std::mutex manager_mutex;
using namespace lifx;

void discovery(Manager manager, bool *bgthread)
{
	while(*bgthread)
	{
		manager_mutex.lock();
		manager.Discover();
		manager.PurgeOldDevices();
		manager.ListGroups();
		manager_mutex.unlock();
		std::this_thread::sleep_for (std::chrono::seconds(5));
	}
}

int main(int argc, const char* argv[]) {
    Manager manager("255.255.255.255");
	bool bgthread = true;
    manager.Initialize();
	std::thread t1(discovery, manager, &bgthread);
	
    if (argc > 1) {
        // set color and quit
		uint16_t arg;
        uint16_t hue, saturation, kelvin;
		float brightness;
		uint32_t fade_time;
		sscanf(argv[1], "%hu", &arg);
		while(1){
		
			if(arg == 1){
				scanf("%hu %hu %f %hu %iu", &hue, &saturation, &brightness, &kelvin, &fade_time);
				if(hue == 3)
				{
					break;
				}
				std::cout << hue << " " << saturation << " " << brightness << " " << kelvin << " " << fade_time << "\n";
				manager.SetColor("Living Room", hue, saturation, brightness, kelvin, fade_time);
				
			} else if(arg == 2) {
				scanf("%hu", &hue);
				if(hue == 1)
				{
					manager_mutex.lock();
					manager.LightsDown("Living Room", true);
					manager_mutex.unlock();
				}
				else if(hue == 2) {
					manager_mutex.lock();
					manager.LightsRestore("Living Room");
					manager_mutex.unlock();
				} else if(hue == 3){
					manager_mutex.lock();
					manager.LightsUp("Living Room", true);
					manager_mutex.unlock();
				} else if(hue == 4) {
					break;
				}
			} 
        }
    }

	bgthread = false;
	t1.join();
	
    return 0;
}
