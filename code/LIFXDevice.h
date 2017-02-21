#pragma once

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
        state = LIFXDeviceState();
        savedState = LIFXDeviceState();
        pending_acks = 0;
	}
	
	bool Expired(unsigned currentTime)
	{
		return ((state.timestamp + ExpiryTime) < currentTime);
	}

	const MacAddress& Address() const { return addr; }
	void Address(const MacAddress& address) { addr = address; }
	
	const uint16_t& SavedHue() const { return savedState.hue; }
	const uint16_t& SavedBrightness() const { return savedState.brightness; }
	const uint16_t& SavedKelvin() const { return savedState.kelvin; }
	const uint16_t& SavedSaturation() const { return savedState.saturation; }
	const uint16_t& SavedPower() const { return savedState.power; }
	
	const uint16_t& Hue() const { return state.hue; }
	const uint16_t& Saturation() const { return state.saturation; }
	const uint16_t& Kelvin() const { return state.kelvin; }
	const uint16_t& Brightness() const { return state.brightness; }
	const uint16_t& Power() const { return state.power; }
	const std::string& Name() const {return name; }

    unsigned SavedTime()
    {
        return savedState.timestamp;
    }
    
	void SaveState()
	{
        savedState = state;
	}
    
    void SaveTime(unsigned timestamp)
    {
        savedState.timestamp = timestamp;
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
	
	void SetAttributes(const std::string& n, const uint16_t& h, const uint16_t& s, const uint16_t& b, const uint16_t& k, const uint16_t& p, const unsigned& t)
	{
		name = n;
		state.Set(h, s, b, k, p, t);
	}
    
	#if 0
    istream& operator>>(istream& is)
    {
        std::string address;
        is >> name;
        is >> address;
        is >> hue;
        is >> saturation;
        is >> brightness;
        is >> kelvin;
        is >> power;
        addr.FromString(address);
        return is;
    }

    ostream& operator<<(ostream& os, const Entry2& en)
    {
        os << name;
        os << addr.ToString();
        os << hue;
        os << saturation;
        os << brightness;
        os << kelvin;
        os << power;
        return os;
    }
    #endif

	std::string ToString() const {
        std::stringstream ret;
		ret << name << "\n";
		ret << "\tMac Address: " << addr.ToString() << "\n";
		ret << "\tHue: " << state.hue << "\n";
		ret << "\tSaturation: " << state.saturation << "\n";
		ret << "\tBrightness: " << state.brightness << "\n";
		ret << "\tKelvin: " << state.kelvin << "\n";
		ret << "\tPower: " << state.power << "\n";
		return ret.str();
	}
protected:
	std::string name;
	MacAddress addr;
	LIFXDeviceState state;
    LIFXDeviceState savedState;

	uint16_t saved_power;
	uint16_t saved_hue;
	uint16_t saved_saturation;
	uint16_t saved_brightness;
	uint16_t saved_kelvin;

    uint32_t pending_acks;
	
	static const unsigned ExpiryTime = 120000;
};
}