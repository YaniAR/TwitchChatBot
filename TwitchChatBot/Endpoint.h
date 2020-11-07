#ifndef ENDPOINT_H
#define ENDPOINT_H

#include <string>
#include <vector>
#include "Socket.h"

class IPEndPoint
{
private:
	Socket::IPVersion m_ipversion{ Socket::IPVersion::Unknown };
	std::string m_hostname{ "" };
	std::string m_ipstring{ "" };
	std::vector<uint8_t> m_ipbytes{};
	unsigned short m_port{ 0 };
public:
	IPEndPoint(const char* ip, unsigned short port);

	sockaddr_in GetSockaddrIPv4();

	Socket::IPVersion GetIpVersion(){ return m_ipversion; }
	std::string GetHostname(){ return m_hostname; }
	std::string getIPString(){ return m_ipstring; }
	std::vector<uint8_t> getIPBytes(){ return m_ipbytes; }
	unsigned short getPort(){ return m_port; }
};

#endif