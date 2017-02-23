#pragma once

#include <iostream>

namespace lifx {
    
class LIFXDeviceState{
public:
    LIFXDeviceState()
    {
        hue = 0;
        saturation = 0;
        brightness = 0;
        kelvin = 0;
        power = 0;
        timestamp = 0;
    }
    
    LIFXDeviceState(uint16_t h, uint16_t s, uint16_t b, uint16_t k, uint16_t p, unsigned t)
    {
        hue = h;
        saturation = s;
        brightness = b;
        kelvin = k;
        power = p;
        timestamp = t;
    }
    
    LIFXDeviceState& operator=(const LIFXDeviceState& other)
    {
        LIFXDeviceState state;
        hue = other.hue;
        saturation = other.saturation;
        brightness = other.brightness;
        kelvin = other.kelvin;
        power = other.power;
        timestamp = other.timestamp;
        
        return *this;
    }
    
    bool operator!=(const LIFXDeviceState& other)
    {
        if((hue != other.hue) ||
            (saturation != other.saturation) ||
            (brightness != other.brightness) ||
            (kelvin != other.kelvin) ||
            (power != other.power))
        {
            return true;
        }
        
        return false;
    }
   
    bool operator>=(const LIFXDeviceState& other)
    {
        if(power > other.power)
        {
            return true;
        }
        else if(power == other.power)
        {
            if(brightness >= other.brightness)
            {
                return true;
            }
        }
        
        return false;
    }
    
    void Set(const uint16_t& h, const uint16_t& s, const uint16_t& b, const uint16_t& k, const uint16_t& p, const unsigned& t)
    {
        hue = h;
        saturation = s;
        brightness = b;
        kelvin = k;
        power = p;
        timestamp = t;
    }
    
    uint16_t hue;
    uint16_t saturation;
    uint16_t brightness;
    uint16_t kelvin;
    uint16_t power;
    unsigned timestamp;
};

class LIFXDevice {
	public:
         
	LIFXDevice(const MacAddress& address = MacAddress())
	{
		name = "Unnamed Device";
		addr = address;
        state = new LIFXDeviceState();
        savedState = new LIFXDeviceState();
        pending_acks = 0;
        discovered = false;
	}
	
	bool Expired(unsigned currentTime)
	{
		return ((state->timestamp + ExpiryTime) < currentTime);
	}

	const MacAddress& Address() const { return addr; }
	void Address(const MacAddress& address) { addr = address; }
    void Name(const std::string& n) { name = n; }
    const std::string& Name() const {return name; }
    void Timestamp(const unsigned& t) { state->timestamp = t; }
    void Discovered(const bool& d) { discovered = d; }
	
    LIFXDeviceState* State() { return state; }
    const bool& Discovered() const { return discovered; }
	const uint16_t& SavedHue() const { return savedState->hue; }
	const uint16_t& SavedBrightness() const { return savedState->brightness; }
	const uint16_t& SavedKelvin() const { return savedState->kelvin; }
	const uint16_t& SavedSaturation() const { return savedState->saturation; }
	const uint16_t& SavedPower() const { return savedState->power; }
    LIFXDeviceState* SavedState() { return savedState; }
    void SavedHue(const uint16_t& h) { savedState->hue = h; }
    void SavedSaturation(const uint16_t& s) { savedState->saturation = s; }
    void SavedBrightness(const uint16_t& b) { savedState->brightness = b; }
    void SavedKelvin(const uint16_t& k) { savedState->kelvin = k; }
    void SavedPower(const uint16_t& p) { savedState->power = p; }
	
	const uint16_t& Hue() const { return state->hue; }
	const uint16_t& Saturation() const { return state->saturation; }
	const uint16_t& Kelvin() const { return state->kelvin; }
	const uint16_t& Brightness() const { return state->brightness; }
	const uint16_t& Power() const { return state->power; }
	

    void Address(const std::string address) 
    { 
        addr.FromString(address); 
    }
    
    unsigned SavedTime()
    {
        return savedState->timestamp;
    }
    
	void SaveState()
	{
        savedState = state;
	}
    
    void SaveTime(unsigned timestamp)
    {
        savedState->timestamp = timestamp;
    }
    
    void ClearPendingAcks()
    {
        pending_acks = 0;
    }
    
    bool HasPendingAcks()
    {
        return pending_acks > 0;
    }
    
    void AddPendingAcks(uint32_t num_acks)
    {
        pending_acks += num_acks;
    }
    
    void SubtractPendingAck()
    {
        pending_acks--;
    }
	
	void SetAttributes(const std::string& n, const uint16_t& h, const uint16_t& s, const uint16_t& b, const uint16_t& k, const uint16_t& p, const unsigned& t, bool d)
	{
		name = n;
		state->Set(h, s, b, k, p, t);
        discovered = d;
	}

	std::string ToString() const {
        std::stringstream ret;
		ret << name << "\n";
		ret << "\tMac Address: " << addr.ToString() << "\n";
		ret << "\tHue: " << state->hue << "\n";
		ret << "\tSaturation: " << state->saturation << "\n";
		ret << "\tBrightness: " << state->brightness << "\n";
		ret << "\tKelvin: " << state->kelvin << "\n";
		ret << "\tPower: " << state->power << "\n";
        ret << "\tSaved Hue: " << savedState->hue << "\n";
		ret << "\tSaved Saturation: " << savedState->saturation << "\n";
		ret << "\tSaved Brightness: " << savedState->brightness << "\n";
		ret << "\tSaved Kelvin: " << savedState->kelvin << "\n";
		ret << "\tSaved Power: " << savedState->power << "\n";
		return ret.str();
	}
protected:
	std::string name;
	MacAddress addr;
	LIFXDeviceState* state;
    LIFXDeviceState* savedState;

	uint16_t saved_power;
	uint16_t saved_hue;
	uint16_t saved_saturation;
	uint16_t saved_brightness;
	uint16_t saved_kelvin;

    uint32_t pending_acks;
    bool discovered;
	
	static const unsigned ExpiryTime = 120000;
};

inline std::istream& operator>>(std::istream& is, LIFXDevice* device)
{
    std::string line;
    
    getline(is, line);
    device->Name(line);
    
    getline(is, line);
    device->Address(line);
    
    getline(is, line);
    device->SavedHue(std::stoi(line));
    
    getline(is, line);
    device->SavedSaturation(std::stoi(line));
    
    getline(is, line);
    device->SavedBrightness(std::stoi(line));
    
    getline(is, line);
    device->SavedKelvin(std::stoi(line));
    
    getline(is, line);
    device->SavedPower(std::stoi(line));
    
    return is;
}

inline std::ostream& operator<<(std::ostream& os, LIFXDevice* device)
{
    os << device->Name() << std::endl;
    os << device->Address().ToString() << std::endl;
    os << device->SavedHue() << std::endl;
    os << device->SavedSaturation() << std::endl;
    os << device->SavedBrightness() << std::endl;
    os << device->SavedKelvin() << std::endl;
    os << device->SavedPower() << std::endl;
    
    return os;
}

}