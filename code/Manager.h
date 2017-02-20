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
		void SetColor(std::string group, uint16_t hue, uint16_t saturation, float brightness, uint16_t kelvin, uint32_t fade_time = 0, bool save = false);
		void SetGroupDeviceAttributes(const MacAddress& target, const std::string& label, const uint16_t& hue, const uint16_t& saturation, const uint16_t& brightness, const uint16_t& kelvin, const uint16_t& power, const unsigned& last_discovered);
		void RestoreColor(std::string group, uint32_t fade_time);
		void LightsRestore(std::string group);
		void LightsUp(std::string group, bool save);
		void LightsDown(std::string group, bool save);
		void AddGroup(const std::string& label, const MacAddress& address);
		void AddGroupDevice(std::string groupName, MacAddress target);
		
		std::shared_ptr<Socket> socket;
		std::map<std::string, Group*> groups;
		unsigned lastRecvTime;
		std::string lastprint;
		static const unsigned timeout = 1000;
};
}
