#include "LIFXDevice.h"

#pragma once

namespace lifx {
class LIFXDevice;
class LIFXGroup {
public:
	LIFXGroup()
	{
		name = "Unnamed Group";
		devices.clear();
	}
	
	LIFXGroup(std::string n)
	{
		name = n;
		devices.clear();
	}
	
	void AddDevice(const MacAddress& address)
	{
		std::string devName = GetDevice(address);
		
		if(devName == "")
		{
			devices[address.ToString()] = new LIFXDevice(address);
		}
	}
	
	const std::string GetDevice(MacAddress address)
	{
		std::string devname = "";
		
		for (auto& it : devices) {
			if(it.second->Address() == address)
			{
				devname = it.second->Name();
				break;
			}
		}
		
		return devname;
	}
	
	bool ContainsDevice(MacAddress address)
	{
		std::string deviceKey = address.ToString();
		return (devices.find(deviceKey) != devices.end());
	}
	
	void SetDeviceAttributes(const MacAddress& target, const std::string& label, const uint16_t& hue, const uint16_t& saturation, const uint16_t& brightness, const uint16_t& kelvin, const uint16_t& power, const unsigned& last_discovered)
	{
		devices[target.ToString()]->SetAttributes(label, hue, saturation, brightness, kelvin, power, last_discovered);
	}
	
	void PurgeOldDevices(unsigned currentTime)
	{
		std::map<std::string, LIFXDevice*>::iterator itr = devices.begin();
		while (itr != devices.end()) {
			if (itr->second->Expired(currentTime)) {
			   itr = devices.erase(itr);
			} else {
			   ++itr;
			}
		}
	}
	
	void RemoveDevice(const MacAddress& address)
	{
		std::map<std::string, LIFXDevice*>::iterator itr = devices.begin();
		while (itr != devices.end()) {
			if (itr->second->Address() == address) {
			   itr = devices.erase(itr);
			} else {
			   ++itr;
			}
		}
	}
#if 0	
	void SaveState()
	{
		for (auto& it: devices)
		{
			it.second->SaveState();
		}
	}
#endif	
	std::string ToString() const {
        std::stringstream ret;
		ret << "\nGroup: " << name << "\n";
		ret << "===============================\n";
		for(const auto& it : devices)
		{
		    ret << "Device: " << it.second->ToString() << "\n\n";
		}
		return ret.str();
	}
	
	std::map<std::string, LIFXDevice*> devices;
protected:
	
	std::string name;
};
}