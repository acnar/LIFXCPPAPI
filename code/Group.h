#include "LIFXDevice.h"

#pragma once

namespace lifx {
class LIFXDevice;
class Group {
public:
	Group()
	{
		name = "Unnamed Group";
		devices.clear();
	}
	
	Group(std::string n)
	{
		name = n;
		devices.clear();
	}
	
	void AddDevice(MacAddress address)
	{
		std::string devName = GetDevice(address);
		
		if(devName == "")
		{
			std::string label = address.ToString();
			devices[address.ToString()] = LIFXDevice(label, address);
		}
	}
	
	std::string GetDevice(MacAddress address)
	{
		std::string label = "";
		
		for (auto& it : devices) {
			if(it.second.GetAddress() == address)
			{
				label = it.second.GetName();
				break;
			}
		}
		
		return label;
	}
	
	bool ContainsDevice(MacAddress address)
	{
		std::string deviceKey = address.ToString();
		return (devices.find(deviceKey) != devices.end());
	}
	
	void SetDeviceAttributes(MacAddress target, std::string label, uint16_t hue, uint16_t saturation, uint16_t brightness, uint16_t kelvin, uint16_t power, unsigned last_discovered)
	{
		std::string deviceKey = target.ToString();
		devices[deviceKey].SetAttributes(label, hue, saturation, brightness, kelvin, power, last_discovered);
	}
	
	void PurgeOldDevices(unsigned currentTime)
	{
		std::map<std::string, LIFXDevice>::iterator itr = devices.begin();
		while (itr != devices.end()) {
			if (itr->second.Expired(currentTime)) {
				//std::cout << "purging device " << itr->second.GetName() << "\n";
			   itr = devices.erase(itr);
			} else {
			   ++itr;
			}
		}
	}
	
	std::string ToString() const {
        std::stringstream ret;
		ret << "\nGroup: " << name << "\n";
		ret << "===============================\n";
		for(const auto& it : devices)
		{
		    ret << "Device: " << it.second.ToString() << "\n\n";
		}
		return ret.str();
	}
	
	std::map<std::string, LIFXDevice> devices;
protected:
	
	std::string name;
};
}