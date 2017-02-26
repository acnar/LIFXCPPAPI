#pragma once
#include <string>
#include <sstream>
#include <cstdint>
#include <iomanip>
#include <cassert>
#include <cstring>
#include <string.h>

namespace lifx {

// from the awesome https://github.com/magicmonkey/lifxjs/blob/master/Protocol.md

#pragma pack(push, 1) 


namespace Payload {

struct StateService {
	enum class Service {
        UDP = 1, TCP = 2
    };
	
	uint8_t service;
	uint32_t port;
};

struct StateHostInfo {
	uint32_t signal;
	uint32_t tx;
	uint32_t rx;
	uint16_t reserved;
};

struct StateHostFirmware {
	uint32_t build[2];
	uint32_t reserved[2];
	uint32_t version;
};

struct StateWifiInfo {
	uint32_t signal;
	uint32_t tx;
	uint32_t rx;
	uint16_t reserved;
};

struct StateWifiFirmware {
	uint32_t build[2];
	uint32_t reserved[2];
	uint32_t version;
};

struct SetPower {
	void Initialize() {
        memset(this, 0, sizeof(*this));
    }
	uint16_t level;
    uint32_t duration;
};

struct StatePower {
	uint16_t level;
};

struct SetLabel {
	char label[32];
};

struct StateLabel {
	char label[32];
};

struct StateVersion {
	uint32_t vendor;
	uint32_t product;
	uint32_t version;
};

struct StateInfo {
	uint32_t time[2];
	uint32_t uptime[2];
	uint32_t downtime[2];
};

struct StateLocation {
	uint8_t location[16];
	char label[32];
	uint32_t updated_at[2];
};

struct StateGroup {
	uint8_t group[16];
	char label[32];
	uint32_t updated_at[2];
};

struct EchoRequest {
	uint8_t payload[64];
};

struct EchoResponse {
	uint8_t payload[64];
};

struct WifiInfo {
    float signal;   // LE
    int tx;         // LE
    int rx;         // LE
    short mcu_temperature;
};

struct BulbLabel {
    char label[32]; // UTF-8 encoded string
};

struct LightStatus {
    uint16_t hue;          // LE
    uint16_t saturation;   // LE
    uint16_t brightness;   // LE
    uint16_t kelvin;       // LE
    uint16_t dim;          // LE?
    uint16_t power;
    char label[32]; // UTF-8 encoded string
    uint64_t tags;
};

struct LightColorRGBW {
    void Initialize() {
        memset(this, 0, sizeof(*this));
    }
    uint16_t blue;
    uint16_t green;
    uint16_t red;
    uint16_t white;
};

struct LightColorHSL {
    void Initialize() {
        memset(this, 0, sizeof(*this));
    }
    uint8_t stream;        // Unknown, potential "streaming" mode toggle? Set to
    // 0x00 for now.
    uint16_t hue;
    uint16_t saturation;
    uint16_t brightness;
    uint16_t kelvin;
    uint32_t fade_time;   // LE Length of fade action, in seconds
};

struct SetDim {
    uint8_t reserved;
    uint8_t brightness; // LE
    uint32_t duration;  // in seconds
};
}

namespace Protocol {
//const uint16_t Send = 0x3400;
const uint16_t Send = 0x400;
}

namespace PacketType {
const uint16_t Invalid = 0xffff;
const uint16_t GetService = 0x0002;
const uint16_t StateService = 0x0003;
const uint16_t GetHostInfo = 0x000c;
const uint16_t StateHostInfo = 0x000d;
const uint16_t GetHostFirmware = 0x000e;
const uint16_t StateHostFirmware = 0x000f;
const uint16_t GetWifiInfo = 0x0010;
const uint16_t StateWifiInfo = 0x0011;
const uint16_t GetWifiFirmware = 0x0012;
const uint16_t StateWifiFirmware = 0x0013;
const uint16_t GetPower = 0x0014;
const uint16_t SetPower = 0x0075;
const uint16_t StatePower = 0x0016;
const uint16_t GetLabel = 0x0017;
const uint16_t SetLabel = 0x0018;
const uint16_t StateLabel = 0x0019;
const uint16_t GetVersion = 0x0020;
const uint16_t StateVersion = 0x0021;
const uint16_t GetInfo = 0x0022;
const uint16_t StateInfo = 0x0023;
const uint16_t Acknowledgement = 0x002d;
const uint16_t GetLocation = 0x0030;
const uint16_t StateLocation = 0x0032;
const uint16_t GetGroup = 0x0033;
const uint16_t StateGroup = 0x0035;
const uint16_t EchoRequest = 0x003A;
const uint16_t EchoResponse = 0x003B;

//const uint16_t GetPanGateway = 0x02;
//const uint16_t PanGatewayState = 0x03;
const uint16_t GetBulbLabel = 0x17;
const uint16_t BulbLabel = 0x19;
const uint16_t GetLightState = 0x65;
const uint16_t SetLightColorHSL = 0x66;
const uint16_t SetDim = 0x68;
const uint16_t SetLightColorRGBW = 0x6a;
const uint16_t LightStatus = 0x6b;
}

struct MacAddress {
    uint8_t address[6];

    MacAddress() {
        for (unsigned i = 0; i < 6; ++i) {
            address[i] = 0;
        }
    }
	
	MacAddress(const MacAddress& other) {
        for (unsigned i = 0; i < 6; ++i) {
            address[i] = other.GetByte(i);
        }
    }

    bool operator==(const MacAddress& other) const {
        for (unsigned i = 0; i < 6; ++i) {
            if (address[i] != other.address[i]) {
                return false;
            }
        }
        return true;
    }
	
	uint8_t GetByte(unsigned i) const
	{
		if(i < 6) {
			return address[i];
		}
		else return 0;
	}
#if 0	
	bool operator<(const MacAddress& l, const MacAddress& r )
	{ 
		uint64_t lwhole, rwhole = 0;
		
		for (unsigned i = 0; i < 6; ++i) {
            lwhole |= (l[i] << (8*i));
			rwhole |= (r[i] << (8*i));
        }
    
		return lwhole < rwhole;
	}
#endif
    bool IsNull() const {
        for (unsigned i = 0; i < 6; ++i) {
            if (address[i] != 0) {
                return false;
            }
        }

        return true;
    }

    std::string ToString() const {
        std::stringstream stream;

        for (unsigned i = 0; i < 6; ++i) {
            stream << std::setw(2) << std::setfill('0') << std::hex;
            stream << (int) address[i];
            if (i < 5) {
                stream << "::";
            }
        }
        return stream.str();
    }

    void FromString(const std::string& str) {
        unsigned fakeAddr[6];
        sscanf(str.c_str(), "%x::%x::%x::%x::%x::%x", &fakeAddr[0],
                &fakeAddr[1], &fakeAddr[2], &fakeAddr[3], &fakeAddr[4],
                &fakeAddr[5]);
        for (unsigned i = 0; i < 6; ++i) {
            address[i] = (uint8_t) fakeAddr[i];
        }
    }
};

class Packet {
    union payload_t {
        //Payload::PanGatewayState panGatewayState;
        Payload::WifiInfo wifiInfo;
        Payload::BulbLabel bulbLabel;
        Payload::LightStatus lightStatus;
        Payload::LightColorRGBW lightColorRGBW;
        Payload::LightColorHSL lightColorHSL;
        Payload::SetDim setDim;
		Payload::EchoRequest echoRequest;
		Payload::EchoResponse echoResponse;
		Payload::StateGroup stateGroup;
		Payload::StateHostFirmware stateHostFirmware;
		Payload::StateHostInfo stateHostInfo;
		Payload::StateInfo stateInfo;
		Payload::StateLabel stateLabel;
		Payload::StateLocation stateLocation;
		Payload::StatePower statePower;
		Payload::StateService stateService;
		Payload::StateVersion stateVersion;
		Payload::StateWifiFirmware stateWifiFirmware;
		Payload::StateWifiInfo stateWifiInfo;
		Payload::SetLabel setLabel;
		Payload::SetPower setPower;
    };

    /* Frame */
    uint16_t size;            
    uint16_t protocol:12;
	uint16_t addressable:1;
	uint16_t tagged:1;		  // set to 1 to send to all devices (target all zeros) otherwise set to 0
	uint16_t origin:2;
    uint32_t source;
	
	/* Frame Address */
    MacAddress target;        // 6 byte device address (MAC address) or zero (0) means all devices
	uint16_t reserved1;
    uint8_t reserved2[6];     // must be zero
    uint8_t res_required:1;   // now set to zero so packets can get dropped - may want to change in future if packets lost
	uint8_t ack_required:1;   // now set to zero so packets can get dropped - may want to change in future if packets lost
	uint8_t reserved3:6;
	uint8_t sequence;	
	
	/* Protocol Header */
    uint64_t reserved4;       
    uint16_t packet_type;  
	uint16_t reserved5;

    /* Payload */       
    payload_t payload;    

public:
    Packet() {
		size = 0;
		protocol = 0;
		reserved1 = 0;
        reserved3 = 0;
		reserved4 = 0;
		reserved5 = 0;
		res_required = 0;
		ack_required = 0;
		addressable = 1;
		origin = 0;
		source = 0; //x31415926;
		sequence = 0;
		
		for(unsigned i = 0; i < 6; i++)
		{
			reserved2[i] = 0;
		}
		
        Initialize(PacketType::Invalid);
    }

    uint16_t GetSize() const {
        return size;
    }

    uint32_t GetSource()const {
		return source;
	}
    
    const payload_t& GetPayload()
    {
        return payload;
    }
    
    void Initialize(uint16_t type, uint8_t t = 1, MacAddress addr = MacAddress(),uint32_t src = 0, uint8_t areq = 0) {
        packet_type = type;
        protocol = Protocol::Send;
		tagged = t;
        target = addr;
        size = sizeof(Packet) - sizeof(payload_t);
		source = src;
		ack_required = areq;
        res_required = 0;
    }

	
    uint16_t GetType() const {
        return packet_type;
    }
	
	Payload::StateService GetStateService() const {
	    assert (GetType() == PacketType::StateService);
        return payload.stateService;
	}

    Payload::StateLabel GetStateLabel() const {
        assert(GetType() == PacketType::StateLabel);
        return payload.stateLabel;
    }
	
	Payload::StateGroup GetStateGroup() const {
        assert(GetType() == PacketType::StateGroup);
        return payload.stateGroup;
    }

    Payload::LightStatus GetLightStatus() const {
        assert(GetType() == PacketType::LightStatus);
        return payload.lightStatus;
    }

    void SetLightColorRGBW(const Payload::LightColorRGBW& lightColor) {
        Initialize(PacketType::SetLightColorRGBW);
        size += sizeof(Payload::LightColorRGBW);
        payload.lightColorRGBW = lightColor;
    }

    void SetLightColorHSL(const Payload::LightColorHSL& lightColor, uint8_t tagged = 1, MacAddress target = MacAddress(), uint8_t ack = 0) {
        Initialize(PacketType::SetLightColorHSL, tagged, target, 0, ack);
        size += sizeof(Payload::LightColorHSL);
        payload.lightColorHSL = lightColor;
    }
	
	void SetPower(const Payload::SetPower& power, uint8_t tagged = 1, MacAddress target = MacAddress(), uint8_t ack = 0) {
		Initialize(PacketType::SetPower, tagged, target, 0, ack);
		size += sizeof(Payload::SetPower);
		payload.setPower = power;
	}

    void SetDim(const Payload::SetDim& setDim) {
        Initialize(PacketType::SetDim);
        size += sizeof(Payload::SetDim);
        payload.setDim = setDim;
    }

    std::string ToString() const {
        std::stringstream ret;
		if(tagged == 1)
		{
			ret << "tagged: 1, ";
		}
        ret << "size: " << size;
        ret << ", packet type: " << "0x" << std::hex << packet_type << std::dec;
		ret << ", addr = " << target.ToString();
        if (packet_type == PacketType::Invalid) {
            ret << " (invalid packet)";
        } else if (packet_type == PacketType::GetService) {
            ret << " (GetService)";
        } else if (packet_type == PacketType::StateService) {
            ret << " (StateService)";
            ret << ", service: " << (int) payload.stateService.service;
            ret << ", port: " << payload.stateService.port;
        } else if (packet_type == PacketType::GetWifiInfo) {
            ret << " (GetWifiInfo)";
        } else if (packet_type == PacketType::StateWifiInfo) {
            ret << " (WifiInfo), signal: " << payload.wifiInfo.signal;
        } else if (packet_type == PacketType::GetLabel) {
            ret << " (GetLabel)";
        } else if (packet_type == PacketType::StateLabel) {
            ret << " (StateLabel), label: " << payload.stateLabel.label;
		} else if (packet_type == PacketType::GetGroup) {
            ret << " (GetGroup)";
        } else if (packet_type == PacketType::StateGroup) {
            ret << " (StateGroup), label: " << (std::string)payload.stateGroup.label;
			ret << ", group: " << payload.stateGroup.group;
			ret << ", updated_at: " << payload.stateGroup.updated_at;
        }else if (packet_type == PacketType::LightStatus) {
            ret << " (LightStatus), label: " << payload.lightStatus.label;
            ret << std::hex;
            ret << ", hue: 0x" << payload.lightStatus.hue;
            ret << ", sat: 0x" << payload.lightStatus.saturation;
            ret << ", brightness: 0x" << payload.lightStatus.brightness;
            ret << ", kelvin: 0x" << payload.lightStatus.kelvin;
            ret << ", dim: 0x" << payload.lightStatus.dim;
            ret << ", power: 0x" << payload.lightStatus.power;
        } else if (packet_type == PacketType::SetLightColorRGBW) {
            ret << " (SetLightColorRGBW)";
        } else if (packet_type == PacketType::SetLightColorHSL) {
            ret << " (SetLightColorHSL)";
        } else if (packet_type == PacketType::GetLightState) {
            ret << " (GetLightState)";
        } else if (packet_type == PacketType::SetDim) {
            ret << " (SetDim)";
        } else if (packet_type == PacketType::Acknowledgement) {
            ret << " (Acknowledgement)";
        } else if (packet_type == PacketType::SetPower) {
            ret << " (Set Power)";
        } else {
            ret << " (unknown)";
        }

#if 0
        if (!target.IsNull()) {
            ret << ", target mac: " << target_mac_address.ToString();
        }

#endif

        return ret.str();
    }

    MacAddress GetTargetMac() const {
        return target;
    }

    void SetTargetMac(const MacAddress& address) {
        target = address;
    }
	
	void SetTagged(uint8_t t)
	{
		tagged = t;
	}
};

#pragma pack(pop) 
}
