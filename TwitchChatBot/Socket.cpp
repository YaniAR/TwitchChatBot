#include "Socket.h"
#include "Endpoint.h"

//IPEndPoint class

IPEndPoint::IPEndPoint(const char* ip, unsigned short port)
	: m_port{ port }
{
	in_addr addr{};

	if (inet_pton(AF_INET, ip, &addr) == 1)
	{
		if (addr.S_un.S_addr != INADDR_NONE)
		{
			m_ipstring = ip;
			m_hostname = ip;

			m_ipbytes.resize(sizeof(ULONG));
			memcpy(&m_ipbytes[0], &addr.S_un.S_addr, sizeof(ULONG));

			m_ipversion = Socket::IPVersion::IPV4;
			return;
		}
	}

	addrinfo hints{};
	hints.ai_family = AF_INET;
	addrinfo* hostinfo{ nullptr };

	// Find ip address of hostname
	if (getaddrinfo(ip, NULL, &hints, &hostinfo) == 0)
	{
		sockaddr_in* hostaddr = reinterpret_cast<sockaddr_in*>(hostinfo->ai_addr);

		m_ipstring.resize(16);
		inet_ntop(AF_INET, &hostaddr->sin_addr, &m_ipstring[0], 16);
		//IPV4 must hold at least 16 characters.

		m_hostname = ip;

		ULONG iplong{ hostaddr->sin_addr.S_un.S_addr };
		m_ipbytes.resize(sizeof(ULONG));
		memcpy(&m_ipbytes[0], &iplong, sizeof(ULONG));

		m_ipversion = Socket::IPVersion::IPV4;
		freeaddrinfo(hostinfo);
	}
}

// Gets IPv4 address
sockaddr_in IPEndPoint::GetSockaddrIPv4()
{
	assert(m_ipversion == Socket::IPVersion::IPV4);

	sockaddr_in addr{};
	addr.sin_family = AF_INET;
	memcpy(&addr.sin_addr, &m_ipbytes[0], sizeof(ULONG));
	addr.sin_port = htons(m_port); // Convert endianess to network settings.

	return addr;
}

// SOCKETS class

Socket::Socket(IPVersion& ipv, SocketHandle& handle)
	: m_ipversion{ ipv }, m_handle{ handle }
{
}

Socket::PResult Socket::SetSocketOption(SocketOption option, BOOL value)
{
	int result{ 0 };
	switch (option)
	{
	// Don't check for right order packets, disables Nadles algorithm
	case SocketOption::TCP_NoDelay:
	{
		result = setsockopt(m_handle, IPPROTO_TCP, TCP_NODELAY, (const char*)&value, sizeof(value));
		break;
	}
	default:
		return PResult::P_FAILURE;
	}

	if (result != 0)
	{
		printf("ERROR #1! %ld\n", WSAGetLastError());
		return PResult::P_FAILURE;
	}

	int val = 10000;
	setsockopt(m_handle, SOL_SOCKET, SO_RCVTIMEO, (char*)&val, sizeof(int));

	return PResult::P_SUCCESS;
}

Socket::PResult Socket::Create()
{
	assert(m_ipversion == IPVersion::IPV4);
	if (m_handle != INVALID_SOCKET)
		return PResult::P_FAILURE;

	m_handle = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (m_handle == INVALID_SOCKET)
	{
		printf("ERROR #2! %ld\n", WSAGetLastError());
		return PResult::P_FAILURE;
	}

	if (SetSocketOption(SocketOption::TCP_NoDelay, TRUE) != PResult::P_SUCCESS)
		return PResult::P_FAILURE;
	return PResult::P_SUCCESS;
}

Socket::PResult Socket::Close()
{
	if (m_handle == INVALID_SOCKET)
		return PResult::P_FAILURE;

	if (closesocket(m_handle) != 0)
	{
		printf("ERROR #3! %ld\n", WSAGetLastError());
		return PResult::P_FAILURE;
	}

	m_handle = INVALID_SOCKET;
	return PResult::P_SUCCESS;
}

// Connect to ipv4_address:port
Socket::PResult Socket::Connect(IPEndPoint ep)
{
	sockaddr_in addr = ep.GetSockaddrIPv4();
	if (connect(m_handle, (sockaddr*)&addr, sizeof(sockaddr_in)) != 0)
	{
		printf("ERROR #4! %ld\n", WSAGetLastError());
		return PResult::P_FAILURE;
	}

	return PResult::P_SUCCESS;
}

// Send data
Socket::PResult Socket::Send(const char* data, int nBytes, int& flag)
{
	if (send(m_handle, data, nBytes, flag) == SOCKET_ERROR)
	{
		printf("ERROR #5! %ld\n", WSAGetLastError());
		return PResult::P_FAILURE;
	}

	return PResult::P_SUCCESS;
}

// Recieve data
Socket::PResult Socket::Recv(char* dest, int nBytes, int& flag, int& bytesRecv)
{
	bytesRecv = recv(m_handle, dest, nBytes, flag);
	switch (bytesRecv)
	{
	case 0:
		return PResult::P_FAILURE;
	case SOCKET_ERROR:
		printf("Listening...\n");
		return PResult::P_FAILURE;
	}

	return PResult::P_SUCCESS;
}