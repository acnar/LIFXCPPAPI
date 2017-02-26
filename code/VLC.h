#pragma once

#include "Socket.h"

namespace lifx {
class VLC {
	public:
	
	VLC(const std::string& ip, const std::string b64AuthString)
	{
		addr = ip;
        try
        {
            socket = std::shared_ptr < Socket
                > (Socket::CreateStream(ip));
        }
        catch(int e)
        {
            std::cout << "Could not connect to VLC \n";
        }
        authString = b64AuthString;
	}

	void RetryConnect()
	{
		socket = std::shared_ptr < Socket
			>(Socket::CreateStream(addr));
	}
	
    void HTTPPost()
    {
        std::string message;
        std::string auth = "";

        if(authString != "")
        {
            auth = "Authorization: Basic " + authString + "\r\n";
        }
        message = "POST /requests/status.xml HTTP/1.1\r\n" + auth + "\r\n";
        
		try 
		{
			socket->Send(message);
		}
		catch (int e)
		{
		}
        
    }
    
	const bool& Connected()
	{
		return connected;
	}

	void Close()
	{
		socket->Close();
	}

	bool GetState(std::string& playstate, std::string& fullscreen)
    {
        int attempts = 0;
        std::string recvstring = "";
        int fs_start, fs_end, s_start, s_end;
        bool success = false;
        
        HTTPPost();

        while(attempts < 3)
        {
			try
			{
				recvstring = socket->Receive(4096);
			}
			catch (int e)
			{
			}
            fs_start = recvstring.find("<fullscreen>");
            if(fs_start > 0)
            {
                success = true;
                break;
            }
            attempts++;
        }
        
        if(success)
        {
            fs_end = recvstring.find("</fullscreen>");
            s_start = recvstring.find("<state>");
            s_end = recvstring.find("</state>");
            
            playstate = recvstring.substr(s_start+7,s_end-s_start-7);
            fullscreen = recvstring.substr(fs_start+12,fs_end-fs_start-12);
        }
        
        return success;
    }
    
private:
	std::shared_ptr<Socket> socket;
    std::string authString;
	std::string addr;
	bool connected = false;
};
}