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
#include "Manager.h"
#include "LIFXDevice.h"

/* 
 * Class for Managing LIFX operations
 */
namespace lifx {

    Manager::Manager(const std::string& broadcastIP) {
        socket = std::shared_ptr < Socket
                > (Socket::CreateBroadcast(broadcastIP));
        groups.clear();
    }
    
    void Manager::ListGroups(void)
	{
		std::string thisprint;
		for (const auto it : groups) {
			thisprint = it.second->ToString();
			if(thisprint != lastprint)
				std::cout << thisprint << "\n";
				lastprint = thisprint;
        }
	}
	
	void Manager::Send(Packet& packet) {
        socket->Send(packet);
    }
	
	void Manager::Broadcast(Packet& packet) {
		packet.SetTagged(1);
        packet.SetTargetMac(MacAddress());
        socket->Send(packet);
    }

	void Manager::SetColor(std::string group, uint16_t hue, uint16_t saturation, float brightness, uint16_t kelvin, uint32_t fade_time, bool save) {
        Packet packet;
		Packet packet2;

        Payload::LightColorHSL lc;
		Payload::SetPower sp;
		
		if(brightness == 0)
		{
			sp.level = 0;
		}
		else
		{
			sp.level = 0xffff;
		}
        lc.Initialize();
        lc.hue = hue;
        lc.saturation = saturation;
        lc.brightness = ((brightness / 100.0) * 0xffff);
		lc.kelvin = kelvin;
        lc.fade_time = fade_time;
		
		for (std::map<std::string, Group*>::iterator it=groups.begin(); it != groups.end(); ++it)
		{
			if(it->first == group)
			{
				for (std::map<std::string, LIFXDevice*>::iterator it2=it->second->devices.begin(); it2 != it->second->devices.end(); ++it2)
				{
					if(save)
					{
						it2->second->SaveState();
					}
					packet2.SetPower(sp, 0, it2->second->Address());
					Send(packet2);
					packet.SetLightColorHSL(lc, 0,it2->second->Address());
					Send(packet);
				}
			}
		}
    }
    
	void Manager::RestoreColor(std::string group, uint32_t fade_time) {
        Packet packet;
		Packet packet2;

        Payload::LightColorHSL lc;
		Payload::SetPower sp;
		sp.Initialize();
        lc.Initialize();
		
		for (std::map<std::string, Group*>::iterator it=groups.begin(); it != groups.end(); ++it)
		{
			if(it->first == group)
			{
				for (std::map<std::string, LIFXDevice*>::iterator it2=it->second->devices.begin(); it2 != it->second->devices.end(); ++it2)
				{
					sp.level = it2->second->SavedPower();
					lc.hue = it2->second->SavedHue();
					lc.saturation = it2->second->SavedSaturation();
					lc.brightness = it2->second->SavedBrightness();
					lc.kelvin = it2->second->SavedKelvin();
					lc.fade_time = fade_time;
		
					packet2.SetPower(sp, 0, it2->second->Address());
					Send(packet2);
					packet.SetLightColorHSL(lc, 0, it2->second->Address());
					Send(packet);
				}
			}
		}
    }
	
	void Manager::LightsRestore(std::string group) {
		RestoreColor(group, 10000);
	}
	
	void Manager::LightsUp(std::string group, bool save) {
		SetColor(group, 0, 0, 100, 5500, 10000, save);
	}
	
	void Manager::LightsDown(std::string group, bool save) {
		SetColor(group, 0, 0, 20, 2500, 10000, save);
	}

    void Manager::Discover() {
		Packet packet;
		lastRecvTime = socket->GetTicks();
		packet.Initialize(PacketType::GetService);
        socket->Send(packet);
        while (1)
		{
			ReadPacket();
			if ((lastRecvTime + timeout) < socket->GetTicks()) {
				break;
			}
		}
    }
	
    void Manager::ReadPacket() {
        Packet packet;
        if (socket->Receive(packet)) {
			lastRecvTime = socket->GetTicks();
			HandleNewPacket(packet);
        }
    }
	
	void Manager::SetGroupDeviceAttributes(const MacAddress& target, const std::string& label, const uint16_t& hue, const uint16_t& saturation, const uint16_t& brightness, const uint16_t& kelvin, const uint16_t& power, const unsigned& last_discovered)
	{
		for (std::map<std::string, Group*>::iterator it=groups.begin(); it != groups.end(); ++it)
		{
			if(it->second->ContainsDevice(target))
			{
				it->second->SetDeviceAttributes(target, label, hue, saturation, brightness, kelvin, power, last_discovered);
			}
		}
	}

    void Manager::AddGroup(const std::string& label, const MacAddress& target)
	{
		if(groups.find(label) == groups.end())
		{
			groups[label] = new Group(label);
		}
		
		// Check if the device adding this group is in any other group
		for(auto it : groups)
		{
			if(it.second->ContainsDevice(target) && label != it.first)
			{
				it.second->RemoveDevice(target);
			}
		}
	}
	
	void Manager::PurgeOldDevices()
	{
		for(auto it : groups)
		{
			it.second->PurgeOldDevices(socket->GetTicks());
		}
	}
	
	void Manager::AddGroupDevice(std::string groupName, MacAddress target)
	{
		AddGroup(groupName, target);
		groups[groupName]->AddDevice(target);
	}
	
    void Manager::HandleNewPacket(const Packet& packet) {
        if (packet.GetType() == PacketType::StateService) {
            Payload::StateService state = packet.GetStateService();
            if (state.service == (int) Payload::StateService::Service::UDP) {
			    Packet p;
			    p.Initialize(PacketType::GetGroup, 0, packet.GetTargetMac());
				Send(p);
            }
        } else if (packet.GetType() == PacketType::StateGroup) {
			Packet p;
			std::string label = (std::string)packet.GetStateGroup().label;
			MacAddress target = packet.GetTargetMac();
			AddGroupDevice(label, target);
			p.Initialize(PacketType::GetLightState, 0, target);
			Send(p);
        } else if (packet.GetType() == PacketType::LightStatus) {
			Payload::LightStatus state = packet.GetLightStatus();
			MacAddress target = packet.GetTargetMac();
			SetGroupDeviceAttributes(target, state.label, state.hue, state.saturation, state.brightness, state.kelvin, state.power, socket->GetTicks());
		}
    }
}