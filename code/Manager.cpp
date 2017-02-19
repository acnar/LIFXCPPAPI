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

namespace lifx {

    Manager::Manager(const std::string& broadcastIP) {
        socket = std::shared_ptr < Socket
                > (Socket::CreateBroadcast(broadcastIP));
    }

    void Manager::Initialize() {
		groups.clear();
		Discover();
    }

    void Manager::ListGroups(void)
	{
		for (const auto& it : groups) {
			std::cout << it.second.ToString() << "\n";
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
		
		for (auto& it : groups) {
			if(it.first == group)
			{
				for(auto& it2 : it.second.devices)
				{
					packet2.SetPower(sp, 0, it2.second.GetAddress());
					Send(packet2);
					packet.SetLightColorHSL(lc, 0,it2.second.GetAddress());
					Send(packet);
					if(save)
					{
						it2.second.SetAttributes(lc.hue, lc.saturation, lc.brightness, lc.kelvin, sp.level);
					}
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
		
		for (auto& it : groups) {
			if(it.first == group)
			{
				for(auto& it2 : it.second.devices)
				{
					sp.level = it2.second.GetPower();
					lc.hue = it2.second.GetHue();
					lc.saturation = it2.second.GetSaturation();
					lc.brightness = it2.second.GetBrightness();
					lc.kelvin = it2.second.GetKelvin();
					lc.fade_time = fade_time;
		
					packet2.SetPower(sp, 0, it2.second.GetAddress());
					Send(packet2);
					packet.SetLightColorHSL(lc, 0,it2.second.GetAddress());
					Send(packet);
				}
			}
		}
    }
	
	
	void Manager::LightsUp(std::string group) {
		RestoreColor(group, 10000);
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
	
	void Manager::SetGroupDeviceAttributes(MacAddress target, std::string label, uint16_t hue, uint16_t saturation, uint16_t brightness, uint16_t kelvin, uint16_t power, unsigned last_discovered)
	{
		for (auto& it : groups) {
			if(it.second.ContainsDevice(target))
			{
				it.second.SetDeviceAttributes(target, label, hue, saturation, brightness, kelvin, power, last_discovered);
			}
		}
	}

    void Manager::AddGroup(std::string label)
	{
		if(groups.find(label) == groups.end())
		{
			groups[label] = Group(label);
		}
	}
	
	void Manager::PurgeOldDevices()
	{
		for(auto& it : groups)
		{
			it.second.PurgeOldDevices(socket->GetTicks());
		}
	}
	
	void Manager::AddGroupDevice(std::string groupName, MacAddress target)
	{
		AddGroup(groupName);
		groups[groupName].AddDevice(target);
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