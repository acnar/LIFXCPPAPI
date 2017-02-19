#pragma once

namespace lifx {
class LIFXDevice {
public:
	LIFXDevice()
	{
		name = "Unnamed Device";
		addr = MacAddress();
		power = 0;
		hue = 0;
		saturation = 0;
		brightness = 0;
		kelvin = 0;
		last_discovered = 0;
	}
	
	LIFXDevice(std::string& n, MacAddress a, uint16_t p=0, uint16_t h=0, uint16_t s=0, uint16_t b=0, uint16_t k=0, unsigned d=0):
		name(n), addr(a)
	{
		power = p;
		hue = h;
		saturation = s;
		brightness = b;
		kelvin = k;
		last_discovered = d;
	}
	
	MacAddress GetAddress()
	{
		return addr;
	}
	
	bool Expired(unsigned currentTime)
	{
		return ((last_discovered + ExpiryTime) < currentTime);
	}
	
	uint16_t GetHue()
	{
		return hue;
	}
	
	uint16_t GetSaturation()
	{
		return saturation;
	}
	
	uint16_t GetBrightness()
	{
		return brightness;
	}
	
	uint16_t GetKelvin()
	{
		return kelvin;
	}
	
	uint16_t GetPower()
	{
		return power;
	}
	
	std::string GetName()
	{
		return name;
	}
	
	void SetName(std::string& n)
	{
		name = n;
	}
	
	void SetAttributes(uint16_t h, uint16_t s, uint16_t b, uint16_t k, uint16_t p)
	{
		hue = h;
		saturation = s;
		brightness = b;
		kelvin = k;
		power = p;
	}
	
	void SetAttributes(std::string n, uint16_t h, uint16_t s, uint16_t b, uint16_t k, uint16_t p, unsigned d)
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
	uint32_t last_discovered;
	static const unsigned ExpiryTime = 30000;
};
}