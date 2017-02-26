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
	
    void Name(const std::string& n) { name = n; }
    const std::string& Name() const {return name; }
    
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
	
	void SetDeviceAttributes(const MacAddress& target, const std::string& label, const uint16_t& hue, const uint16_t& saturation, const uint16_t& brightness, const uint16_t& kelvin, const uint16_t& power, const unsigned& last_discovered, const bool& discovered)
	{
		devices[target.ToString()]->SetAttributes(label, hue, saturation, brightness, kelvin, power, last_discovered, discovered);
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
    
    void RefreshTimestamps(unsigned time)
    {
        for(auto& it : devices)
        {
            it.second->Timestamp(time);
        }
    }

    void AddDevice(LIFXDevice* device)
    {
        std::string label = device->Address().ToString();
        if(devices.find(label) == devices.end())
		{
			devices[label] = device;
		}
    }
    
    bool DiscoveryDone()
    {
        bool done = true;
		if (!devices.empty())
		{
			for (const auto& it : devices)
			{
				if (it.second->Discovered() == false)
				{
					done = false;
					break;
				}
			}
		}
		else
		{
			done = false;
		}
        
        return done;
    }
    
    bool HasGlobalSetting(const LIFXDeviceState& state)
    {
        bool has = true;
        for(const auto& it: devices)
        {
            if(*it.second->State() != state)
            {
                has = false;
                break;
            }
        }
        return has;
    }
    
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

    inline std::istream& operator>>(std::istream& is, LIFXGroup* group)
    {
        LIFXDevice* device;
        std::string line;
        int num_devs;
        int dev = 0;
        if(is)
        {
            getline(is, line);
            if(line != "")
            {
                group->Name(line);
            }
        }
        if(is)
        {
            getline(is, line);
            num_devs = std::stoi(line);
        }
        while(is && (dev < num_devs))
        {
            device = new LIFXDevice();
            is >> device;
            group->AddDevice(device);
            dev++;
        }
        
        return is;
    }

    inline std::ostream& operator<<(std::ostream& os, LIFXGroup* group)
    {
        os << group->Name() << std::endl;
        os << group->devices.size() << std::endl;
        for(const auto& it: group->devices)
        {
            os << it.second;
        }
        return os;
    }
}