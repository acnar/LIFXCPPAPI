#include <string>
#include "LIFXGroup.h"

#pragma once

namespace lifx {
#define MAX_CONFIGS 10

class Config {
    public:
    Config(const std::string& n)
    {
		name = n;
        hue = 0;
        saturation = 0;
        brightness = 0;
        kelvin = 0;
        power = 0;
        fade_time = 0;
        restore_time = 0;
    }
    
    std::string ToString() const
    {
        std::stringstream ret;
		ret << "Name: " << name << "\n";
		ret << "Hue: " << hue << "\n";
		ret << "Saturation: " << saturation << "\n";
        ret << "Brightness: " << brightness << "\n";
        ret << "Kelvin: " << kelvin << "\n";
        ret << "Power: " << power << "\n";
        ret << "Fade Time: " << fade_time << "\n";
        ret << "Restore Time: " << restore_time << "\n";
		return ret.str();
    }
    
	std::string name;
    uint16_t hue;
    uint16_t saturation;
    uint16_t brightness;
    uint16_t kelvin;
    uint16_t power;
    uint32_t fade_time;
    uint32_t restore_time;
    
};

class Manager {
	private:
		
		void ReadPacket();
		void HandleNewPacket(const Packet& packet);
		
	public:
        typedef  bool (LIFXDevice::*LIFXDeviceFn)(void); 
		Manager(const std::string& broadcastIP);
		void Discover();
		void ListGroups();
		void PurgeOldDevices();
		void Send(Packet& packet);
		void Broadcast(Packet& packet);
		void SetGroupDeviceAttributes(const MacAddress& target, const std::string& label, const uint16_t& hue, const uint16_t& saturation, const uint16_t& brightness, const uint16_t& kelvin, const uint16_t& power, const unsigned& last_discovered, bool discovered);
		void LightsRestore(std::string group);
		bool LightsDown(std::string group, bool save);
		void AddGroup(const std::string& label, const MacAddress& address);
        void AddGroup(LIFXGroup* group);
		void AddGroupDevice(std::string groupName, MacAddress target);
        void SubtractGroupDeviceAck(const MacAddress& target);
        void WaitForPackets(LIFXDevice* device, LIFXDeviceFn terminator, bool condition, unsigned to);
		bool SetColorAndPower(LIFXDevice* target, bool ack, bool save, const LIFXDeviceState* state, uint32_t fade_time);
        void ReadDevices(const std::string& fname);
        void WriteDevices(const std::string& fname);
        bool DiscoveryDone(const std::string& group);
		std::string StripNewline(std::string str);
		void SetLightState(int state);
		void SaveGroup(std::string group);
        void ReadConfig();
		void Close();
        
		std::shared_ptr<Socket> socket;
		std::map<std::string, LIFXGroup*> groups;
		Config* configs[MAX_CONFIGS];
		int num_configs;
		unsigned lastRecvTime;
		std::string lastprint;
		static const unsigned timeout = 1000;
        int activeConfigNum;
		std::string controlGroup;
		std::string vlcIP;
		std::string vlcUname;
		std::string vlcPass;
		int lightsStartOn;
		bool savePending;
		int lightState;
};
}
