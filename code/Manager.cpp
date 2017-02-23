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
 * Class for managing LIFX devices.
 */
namespace lifx {

    Manager::Manager(const std::string& broadcastIP) {
        socket = std::shared_ptr < Socket
                > (Socket::CreateBroadcast(broadcastIP));
        groups.clear();
    }
    
    void Manager::ListGroups(void)
	{
		std::string thisprint = "";
		for (const auto it : groups) {
			thisprint += it.second->ToString() + "\n";
        }
        
        if(thisprint != lastprint)
        {
            std::cout << thisprint;
            lastprint = thisprint;
        }
	}
	
	void Manager::Send(Packet& packet) {
        //std::cout << "TX " << packet.ToString() << "\n";
        socket->Send(packet);
    }
	
	void Manager::Broadcast(Packet& packet) {
		packet.SetTagged(1);
        packet.SetTargetMac(MacAddress());
        socket->Send(packet);
    }

    void Manager::SubtractGroupDeviceAck(const MacAddress& target)
    {
        for(auto it : groups)
		{
			if(it.second->ContainsDevice(target))
			{
				it.second->devices[target.ToString()]->SubtractPendingAck();
			}
		}
    }
    
    void Manager::SetColorAndPower(LIFXDevice* target, bool ack, bool save, const LIFXDeviceState* state, uint32_t fade_time)
    {
        Packet color;
		Packet power;

        Payload::LightColorHSL color_payload;
		Payload::SetPower power_payload;
        
		if(state->brightness == 0)
		{
			power_payload.level = 0;
		}
		else
		{
			power_payload.level = state->power;
		}
        power_payload.duration = fade_time;
        color_payload.Initialize();
        color_payload.hue = state->hue;
        color_payload.saturation = state->saturation;
        color_payload.brightness = state->brightness;
		color_payload.kelvin = state->kelvin;
        color_payload.fade_time = fade_time;
        power.SetPower(power_payload, 0, target->Address(),1);
        color.SetLightColorHSL(color_payload, 0, target->Address(),1);
        uint32_t retries = 15;
                
        if(save)
        {
            if(target->SavedTime() < socket->GetTicks())
            {
                target->SaveState();
            }
        }
        
        while(retries > 0)
        {
            target->ClearPendingAcks();
            target->AddPendingAcks(2);
            
            if(power_payload.level != 0)
            {
                /* Turn on the power first */
                Send(power);
            }
            Send(color);
            if(power_payload.level == 0)
            {
                /* Turn off the power last */
                Send(power);
            }
        
            if(ack)
            {
                WaitForPackets(target, LIFXDevice::HasPendingAcks, false, 200);
                
                if(!target->HasPendingAcks())
                {
                    break;
                }
                
                retries --;
            }
            else
            {
                break;
            }
        }
        
        target->SaveTime(socket->GetTicks() + color_payload.fade_time);
        
        if(retries == 0)
        {
            perror("Unable to Set Color and/or Power\n");
        }
    }
    
    void Manager::ReadDevices(const std::string& fname) {
        std::ifstream file(fname);
        std::string name;
        LIFXGroup* group;
         
        while(file)
        {
            group = new LIFXGroup();
            file >> group;
            if(group->Name() != "Unnamed Group")
            {
                group->RefreshTimestamps(socket->GetTicks());
                AddGroup(group);
            }
            else
            {
                free(group);
            }
        }
        file.close();
    }

    void Manager::WriteDevices(const std::string& fname) {
        std::ofstream file(fname);
        for (const auto& it : groups) {
            if (it.first.length() > 0) {
                file << it.second;
            }
        }
        file.close();
    }
	
    bool Manager::DiscoveryDone(const std::string& group)
    {
        return groups[group]->DiscoveryDone();
    }
    
	void Manager::LightsRestore(std::string group) {
        
		for (std::map<std::string, LIFXGroup*>::iterator it=groups.begin(); it != groups.end(); ++it)
		{
			if(it->first == group)
			{
				for (std::map<std::string, LIFXDevice*>::iterator it2=it->second->devices.begin(); it2 != it->second->devices.end(); ++it2)
				{
					
                    SetColorAndPower(it2->second, 1, false, 
                                     it2->second->SavedState(),
                                     10000);
				}
			}
		}
	}
	
	void Manager::LightsDown(std::string group, bool save) {
        
        LIFXDeviceState dimState = LIFXDeviceState(0, 0, 13107, 2500, 0xffff, 0);
        
        if(!DiscoveryDone(group))
        {
            return;
        }
       
        for (std::map<std::string, LIFXGroup*>::iterator it=groups.begin(); it != groups.end(); ++it)
		{
			if(it->first == group)
			{
				for (std::map<std::string, LIFXDevice*>::iterator it2=it->second->devices.begin(); it2 != it->second->devices.end(); ++it2)
				{
                    dimState.brightness = 13107;
                    if(it2->second->Power() >= dimState.power)
                    {
                        if(it2->second->Brightness() < dimState.brightness)
                        {
                            dimState.brightness = it2->second->Brightness();
                        }
                        SetColorAndPower(it2->second, true, false, &dimState, 10000);
                    }
				}
			}
		}
	}

    void Manager::Discover() {
		Packet packet;
		lastRecvTime = socket->GetTicks();
		packet.Initialize(PacketType::GetService);
        Send(packet);
        WaitForPackets(nullptr, nullptr, false, timeout);
        PurgeOldDevices();
    }
	
    void Manager::WaitForPackets(LIFXDevice* device, LIFXDeviceFn terminator, bool condition, unsigned to)
    {
        unsigned lastRecvTime = socket->GetTicks();
        
        while (1)
		{
			ReadPacket();
            if(((terminator != nullptr) && ((device->*terminator)() == condition)))
            {
                break;
            }
			if ((lastRecvTime + to) < socket->GetTicks())
            {
				break;
			}
		}
    }
    
    void Manager::ReadPacket() {
        Packet packet;
        if (socket->Receive(packet)) {
            //std::cout << "RX " << packet.ToString() << "\n";
			lastRecvTime = socket->GetTicks();
			HandleNewPacket(packet);
        }
    }
	
	void Manager::SetGroupDeviceAttributes(const MacAddress& target, const std::string& label, const uint16_t& hue, const uint16_t& saturation, const uint16_t& brightness, const uint16_t& kelvin, const uint16_t& power, const unsigned& last_discovered, bool discovered)
	{
		for (std::map<std::string, LIFXGroup*>::iterator it=groups.begin(); it != groups.end(); ++it)
		{
			if(it->second->ContainsDevice(target))
			{
				it->second->SetDeviceAttributes(target, label, hue, saturation, brightness, kelvin, power, last_discovered, discovered);
			}
		}
	}

    void Manager::AddGroup(LIFXGroup* group)
    {
        std::string label = group->Name();
        if(groups.find(label) == groups.end())
		{
			groups[label] = group;
		}
    }
    
    void Manager::AddGroup(const std::string& label, const MacAddress& target)
	{
		if(groups.find(label) == groups.end())
		{
			groups[label] = new LIFXGroup(label);
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
        std::map<std::string, LIFXGroup*>::iterator itr = groups.begin();
		while (itr != groups.end()) {
			if(itr->second->devices.size() == 0) {
			   itr = groups.erase(itr);
			} else {
			   ++itr;
			}
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
			SetGroupDeviceAttributes(target, state.label, state.hue, state.saturation, state.brightness, state.kelvin, state.power, socket->GetTicks(), true);
		} else if (packet.GetType() == PacketType::Acknowledgement){
            MacAddress target = packet.GetTargetMac();
            SubtractGroupDeviceAck(target);
        }
    }
}