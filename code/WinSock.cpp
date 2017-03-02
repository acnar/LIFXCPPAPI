#ifdef WIN32

#include "Socket.h"
#include <iostream>
#include <cassert>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "Packet.h"

#pragma comment(lib, "Ws2_32.lib")

namespace lifx {
	class Winsock : public Socket {
	public:
		Winsock(const std::string& ip, uint16_t port, bool broadcast) {
			int result;
			if (!WinsockInitialized())
			{
				WSADATA wsaData;
				if (WSAStartup(MAKEWORD(2, 2), &wsaData) != NO_ERROR)
				{
					perror("WSAStartup failed!\n");
				}
			}
			
			if (broadcast) {
				socket_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
			} else {
				socket_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			}
			if (socket_ == INVALID_SOCKET)
			{
				perror("Socket initialization failed!\n");
			}

			memset(&from, 0, sizeof(from));
			from.sin_family = AF_INET;
			from.sin_addr.s_addr = INADDR_ANY;
			from.sin_port = htons(port);

			memset(&dest, 0, sizeof(dest));
			dest.sin_family = AF_INET;
			dest.sin_addr.S_un.S_addr = inet_addr(ip.c_str());
			dest.sin_port = htons(port);

			u_long nonblock = 1;
			ioctlsocket(socket_, FIONBIO, &nonblock);

			if (broadcast)
			{
				const char * broadcastEnable = "1";
				setsockopt(socket_, SOL_SOCKET, SO_BROADCAST, broadcastEnable,
					sizeof(broadcastEnable));

				result = bind(socket_, (SOCKADDR*)&from, sizeof(from));
				if (result == SOCKET_ERROR)
				{
					perror("Socket bind failed\n");
				}
			} 
			else
			{  
				int v = connect(socket_, (sockaddr*)&dest, sizeof(dest));
				if (v != 0)
				{
					// expected to hit this case with non blocking socket since
					// connecting may take some time.
				}
			}

		}

		bool WinsockInitialized()
		{
			SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
			if (s == INVALID_SOCKET && WSAGetLastError() == WSANOTINITIALISED) {
				return false;
			}

			closesocket(s);
			return true;
		}

        void Send(const std::string& data) {
            int sent;
            const char * message = data.c_str();
			int message_len = strlen(message);
           
			if (send(socket_, message, message_len, 0) <= 0)
			{
				throw 0;
			}
		}
        
		std::string Receive(int bytes)
		{
			std::string output(bytes, 0);
			int r = recv(socket_, &output[0], bytes - 1, 0);
			
			if (r == 0)
				printf("VLC Connection closed\n");
			else
			{
				int last_error = WSAGetLastError();
				//printf("recv failed: %d\n", last_error);
				if ((last_error == WSAENOTCONN) || 
					(last_error == WSAECONNRESET) ||
					(last_error == WSAECONNABORTED))
				{
					throw 0;
				}
			}
			
		
            return output;
        }   
    
		void Send(const Packet& packet) {
			int sent = sendto(socket_,(const char*) &packet , packet.GetSize(), 0, (SOCKADDR*) &dest, sizeof(dest));
			if (sent != packet.GetSize()) {
				std::cerr << WSAGetLastError() << " " << sent << " - UDP send error\n";
			}
			assert(sent == packet.GetSize());
		}

		bool Receive(Packet& packet) {
			//unsigned long nBytesAvailable;
			//if ( ioctlsocket(socket_, FIONREAD, &nBytesAvailable) != SOCKET_ERROR )
			//{
			//	if (nBytesAvailable == 0) {
			//		return false;
			//	}
			//}
			int recvd = 0;
			try{
				int fromLen = sizeof(from);

				recvd = recvfrom(socket_, (char*)&packet, sizeof(packet), 0, (SOCKADDR*)&from, &fromLen);
			}
			catch(int e)
			{

			}
			
			return (recvd > 0);
		}

		void Close(bool final)
		{
			closesocket(socket_);
			WSACleanup();
		}

		unsigned GetTicks() const {
			return GetTickCount();
		}

	protected:
		SOCKET socket_;
		sockaddr_in from;
		sockaddr_in dest;
	};

	Socket* Socket::CreateBroadcast(const std::string& broadcastIP, uint16_t port) {
		return new Winsock(broadcastIP, port, true);
	}

	Socket* Socket::CreateStream(const std::string& ip, uint16_t port) {
		return new Winsock(ip, port, false);
	}
}

#endif
