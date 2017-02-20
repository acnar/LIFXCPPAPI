#pragma once

namespace lifx {
class LIFXDevice {
	public:
	
	LIFXDevice(const MacAddress& address = MacAddress())
	{
		name = "Unnamed Device";
		addr = address;
		power = 0;
		hue = 0;
		saturation = 0;
		brightness = 0;
		kelvin = 0;
		last_discovered = 0;
		saved_hue = 0;
		saved_saturation = 0;
		saved_kelvin = 0;
		saved_brightness = 0;
		saved_power = 0;
	}
	
	bool Expired(unsigned currentTime)
	{
		return ((last_discovered + ExpiryTime) < currentTime);
	}

	const MacAddress& Address() const { return addr; }
	void Address(const MacAddress& address) { addr = address; }
	
	const uint16_t& SavedHue() const { return saved_hue; }
	const uint16_t& SavedBrightness() const { return saved_brightness; }
	const uint16_t& SavedKelvin() const { return saved_kelvin; }
	const uint16_t& SavedSaturation() const { return saved_saturation; }
	const uint16_t& SavedPower() const { return saved_power; }
	
	const uint16_t& Hue() const { return hue; }
	const uint16_t& Saturation() const { return saturation; }
	const uint16_t& Kelvin() const { return kelvin; }
	const uint16_t& Brightness() const { return brightness; }
	const uint16_t& Power() const { return power; }
	const std::string& Name() const {return name;}

	void SaveState()
	{
		saved_brightness = brightness;
		saved_hue = hue;
		saved_kelvin = kelvin;
		saved_power = power;
		saved_saturation = saturation;
	}
	
	void SetAttributes(const std::string& n, const uint16_t& h, const uint16_t& s, const uint16_t& b, const uint16_t& k, const uint16_t& p, const unsigned& d)
	{
		name = n;
		hue = h;
		saturation = s;
		brightness = b;
		kelvin = k;
		power = p;
		last_discovered = d;
	}
	
	std::string ToString() const {
        std::stringstream ret;
		ret << name << "\n";
		ret << "\tMac Address: " << addr.ToString() << "\n";
		ret << "\tHue: " << hue << "\n";
		ret << "\tSaturation: " << saturation << "\n";
		ret << "\tBrightness: " << brightness << "\n";
		ret << "\tKelvin: " << kelvin << "\n";
		ret << "\tPower: " << power << "\n";
		return ret.str();
	}
protected:
	std::string name;
	MacAddress addr;
	uint16_t power;
	uint16_t hue;
	uint16_t saturation;
	uint16_t brightness;
	uint16_t kelvin;

	uint16_t saved_power;
	uint16_t saved_hue;
	uint16_t saved_saturation;
	uint16_t saved_brightness;
	uint16_t saved_kelvin;

	uint32_t last_discovered;
	
	static const unsigned ExpiryTime = 120000;
};
}