#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <string>
#include <sstream>
#include <memory>
#include <list>
#include <map>
#include <fstream>
#include <cstring>
#include "Socket.h"
#include "Packet.h"
#include "Color.h"
#include "Manager.h"

using namespace lifx;

int main(int argc, const char* argv[]) {
    Manager manager("255.255.255.255");
    manager.Initialize();
	std::cout << "discovery done\n";
    //manager.ReadBulbs("cache");
    //std::cout << "numargs =" << argc << "\n";
	//manager.GetLightState();
	manager.ListGroups();
	
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
				std::cout << hue << " " << saturation << " " << brightness << " " << kelvin << " " << fade_time << "\n";
				manager.SetColor("Living Room", hue, saturation, brightness, kelvin, fade_time);
			} else if(arg == 2) {
				scanf("%hu", &hue);
				if(hue == 1)
				{
					manager.LightsDown("Living Room", true);
				}
				else if(hue == 2) {
					manager.LightsUp("Living Room");
				}
			}
        }
    }
	
	while(1)
	{
		manager.Discover();
		manager.PurgeOldDevices();
		//manager.ListGroups();
	}

    return 0;
}
