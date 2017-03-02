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
		activeConfigNum = 0;
		controlGroup = "";
		lightsStartOn = 0;
		num_configs = 1; // one config reserved for dimming 
		vlcUname = "";
		vlcPass = "";
		savePending = false;
		lightState = LIGHTS_RESTORED;
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
	
	void Manager::SetLightState(int state)
	{
		//std::cout << "Setting all states to " << state << "\n";
		lightState = state;
		for (auto it : groups[controlGroup]->devices)
		{
			it.second->SetLightState(state);
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
    
    bool Manager::SetColorAndPower(LIFXDevice* target, bool ack, bool save, const LIFXDeviceState* state, uint32_t fade_time)
    {
        Packet color;
		Packet power;
		bool success = true;

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
			//std::cout << "BKP" << target->Name() << " " << color_payload.brightness << " " << color_payload.kelvin << " " << power_payload.level << "\n";
        
            if(ack)
            {
				//std::cout << "waiting for acks\n";
                WaitForPackets(target, &LIFXDevice::HasPendingAcks, false, 200);
                
                if(!target->HasPendingAcks())
                {
					//std::cout << "ack success\n";
                    break;
                }
                
                retries --;
            }
            else
            {
                break;
            }
        }
        
        if(retries == 0)
        {
            perror("Unable to Set Color and/or Power\n");
			success = false;
			//std::cout << "acks failed\n";
        }
		else
		{
			// okay to save now assuming that discovery thread has not done discovery since
			// sending new config (should be waiting for manager lock)
			if (save)
			{
				// SavedTime will have been set as the time of save plus the length
				// of the action 
				if (target->SavedTime() < socket->GetTicks())
				{
					//std::cout << "saved time = " << target->SavedTime() << " curtime = " << socket->GetTicks() << "\n";
					//std::cout << target->State()->ToString();
					target->SaveState();
				}
			}
		}

		// Set the saved time as the current time plus the length of time that the new state will take to be applied.
		target->SaveTime(socket->GetTicks() + color_payload.fade_time + 5000);

		return success;
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

	std::string Manager::StripNewline(std::string str)
	{
		if (str.length() > 0)
		{
			if (str[str.length() - 1] == '\n' || str[str.length() - 1] == '\r')
			{
				return str.substr(0, str.length() - 1);
			}
		}
		return str;
	}

    void Manager::ReadConfig()
    {
        std::ifstream file("config");
        std::string line;
        Config* config;
        
        while(file)
        {
            getline(file, line);
			if (num_configs < MAX_CONFIGS)
			{
				if (line[0] == 'N')
				{
					std::string name = StripNewline(line.substr(3, -1));
					config = new Config(name);
					configs[num_configs++] = config;
				}
				if (line[0] == 'H')
				{
					config->hue = std::stoi(line.substr(3, -1));
				}
				if (line[0] == 'S')
				{
					config->saturation = std::stoi(line.substr(3, -1));
				}
				if (line[0] == 'B')
				{
					config->brightness = std::stoi(line.substr(3, -1));
				}
				if (line[0] == 'K')
				{
					config->kelvin = std::stoi(line.substr(3, -1));
				}
				if (line[0] == 'P')
				{
					config->power = std::stoi(line.substr(3, -1));
				}
				if (line[0] == 'F')
				{
					config->fade_time = std::stoi(line.substr(4, -1));
				}
				if (line[0] == 'R')
				{
					config->restore_time = std::stoi(line.substr(4, -1));
				}
			}
			else
			{
				perror("Too many configs in config file\n.");
			}
			if (line[0] == 'I')
			{
				vlcIP = StripNewline(line.substr(4, -1));
			}
			if (line[0] == 'G')
			{
				controlGroup = StripNewline(line.substr(3, -1));
			}
			if (line[0] == 'C')
			{
				activeConfigNum = std::stoi(line.substr(3, -1));
			}
			if (line[0] == 'V' && line[1] == 'U')
			{
				vlcUname = StripNewline(line.substr(4, -1));
			}
			if (line[0] == 'V' && line[1] == 'P')
			{
				vlcPass = StripNewline(line.substr(4, -1));
			}
			if (line[0] == 'L')
			{
				lightsStartOn = std::stoi(line.substr(3, -1));
			}
        }
        
        file.close();
        
		if (activeConfigNum >= num_configs)
		{
			// override invalid config
			activeConfigNum = 0;
		}
		if (num_configs == 0)
		{
			std::cout << "Error, no configs found\n";
			exit(1);
		}
        for(int i = 1; i < num_configs; i++)
        {
            std::cout << configs[i]->ToString() << "\n";
        }
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
		bool done = false;
		if (!groups.empty())
		{
			done = groups[group]->DiscoveryDone();
		}
		return done;
    }
    
	void Manager::LightsRestore(std::string group) {
        
		bool success = true;

		for (std::map<std::string, LIFXGroup*>::iterator it=groups.begin(); it != groups.end(); ++it)
		{
			if(it->first == group)
			{
				for (std::map<std::string, LIFXDevice*>::iterator it2=it->second->devices.begin(); it2 != it->second->devices.end(); ++it2)
				{
					uint32_t restore_time = 0;
					if (activeConfigNum != 0) restore_time = configs[activeConfigNum]->restore_time;
					bool result = SetColorAndPower(it2->second, true, false,
												it2->second->SavedState(),
												restore_time);
					if (!result)
					{
						success = false;
					}
					else
					{
						it2->second->SetLightState(LIGHTS_RESTORED);
					}
				}
			}
		}
		if (success)
		{
			lightState = LIGHTS_RESTORED;
		}
	}
	
	bool Manager::LightsDown(std::string group, bool save) {
		
		bool success = true;
        LIFXDeviceState dimState = LIFXDeviceState(configs[activeConfigNum]->hue, 
                                                    configs[activeConfigNum]->saturation, 
                                                    configs[activeConfigNum]->brightness, 
                                                    configs[activeConfigNum]->kelvin, 
                                                    configs[activeConfigNum]->power, 
                                                    0);
		//std::cout << "Lights Down\n";
		bool disc = DiscoveryDone(group);
        if(disc == false)
        {
            return false;
        }
	
		//std::cout << "Discovery Done\n";
        for (std::map<std::string, LIFXGroup*>::iterator it=groups.begin(); it != groups.end(); ++it)
		{
			if(it->first == group)
			{
				for (std::map<std::string, LIFXDevice*>::iterator it2=it->second->devices.begin(); it2 != it->second->devices.end(); ++it2)
				{
					LIFXDeviceState* compareState;
					bool save = true;
					if (it2->second->LightState() != LIGHTS_DOWN) // it's possible this light's state was changed but the global state did not
					{
						if (it2->second->LightState() == LIGHTS_CONFIG_CHANGED)
						{
							compareState = it2->second->SavedState();
							if (it2->second->PrevLightState() != LIGHTS_RESTORED)
							{
								save = false;
							}
						}
						else
						{
							compareState = it2->second->State();
						}

						dimState.brightness = configs[activeConfigNum]->brightness;
						if ((compareState->power != 0) || (dimState.power != 0))
						{
							if ((compareState->power >= dimState.power))
							{
								if (compareState->brightness < dimState.brightness)
								{
									dimState.brightness = compareState->brightness;
								}

								if (SetColorAndPower(it2->second, true, save, &dimState, configs[activeConfigNum]->fade_time))
								{
									it2->second->SetLightState(LIGHTS_DOWN);
								}
								else
								{
									success = false;
								}
							}
						}
					}
				}
			}
		}

		if (success)
		{
			// set the global light state
			lightState = LIGHTS_DOWN;
		}
        
        return success;
	}

	void Manager::SaveGroup(std::string group)
	{
		for (std::map<std::string, LIFXGroup*>::iterator it = groups.begin(); it != groups.end(); ++it)
		{
			if (it->first == group)
			{
				for (std::map<std::string, LIFXDevice*>::iterator it2 = it->second->devices.begin(); it2 != it->second->devices.end(); ++it2)
				{
					it2->second->SaveState();
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

		if (savePending)
		{
			if (DiscoveryDone(controlGroup))
			{
				SaveGroup(controlGroup);
				savePending = false;
			}
		}
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

	void Manager::Close() {
		socket->Close(true);
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
			    p.Initialize(PacketType::GetGroup, 0, packet.GetTargetMac(), 0, 1);
				Send(p);
            }
        } else if (packet.GetType() == PacketType::StateGroup) {
			Packet p;
			std::string label = (std::string)packet.GetStateGroup().label;
			MacAddress target = packet.GetTargetMac();
			AddGroupDevice(label, target);
			p.Initialize(PacketType::GetLightState, 0, target, 0, 1);
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