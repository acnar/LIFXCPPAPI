#include <string>
#include "Group.h"

#pragma once

namespace lifx {

class Manager {
	private:
		
		void ReadPacket();
		void HandleNewPacket(const Packet& packet);
		
	public:
	    Manager(const std::string& broadcastIP);
		void Initialize();
		void Discover();
		void ListGroups();
		void PurgeOldDevices();
		void Send(Packet& packet);
		void Broadcast(Packet& packet);
		//void SetGroupColor(std::string groupName, uint16_t hue, uint16_t saturation, float brightness, uint16_t kelvin, uint32_t fade_time = 0);
		void SetColor(std::string group, uint16_t hue, uint16_t saturation, float brightness, uint16_t kelvin, uint32_t fade_time = 0, bool save = false);
		void SetGroupDeviceAttributes(MacAddress target, std::string label, uint16_t hue, uint16_t saturation, uint16_t brightness, uint16_t kelvin, uint16_t power, unsigned last_discovered);
		void RestoreColor(std::string group, uint32_t fade_time);
		void LightsUp(std::string group);
		void LightsDown(std::string group, bool save);
		void AddGroup(std::string label);
		void AddGroupDevice(std::string groupName, MacAddress target);
		
		std::shared_ptr<Socket> socket;
		std::map<std::string, Group> groups;
		std::map<std::string, MacAddress> ungroupedDevices;
		unsigned lastRecvTime;
		static const unsigned timeout = 1000;
};
}
