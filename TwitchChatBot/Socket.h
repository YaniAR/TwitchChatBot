#ifndef SOCKET_H
#define SOCKET_H

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <iostream>
#include <assert.h>

class IPEndPoint;

typedef SOCKET SocketHandle;

class Socket
{
public:
	enum class PResult
	{
		P_FAILURE,
		P_SUCCESS,
	};

	enum class IPVersion
	{
		Unknown,
		IPV4,
		IPV6
	};

	enum class SocketOption
	{
		TCP_NoDelay
	};
private:
	IPVersion m_ipversion{ IPVersion::IPV4 };
	SocketHandle m_handle{ INVALID_SOCKET };
public:
	Socket() = default;
	Socket(IPVersion& ipv, SocketHandle& handle);

	PResult SetSocketOption(SocketOption option, BOOL value);
	PResult Create();
	PResult Close();
	PResult Connect(IPEndPoint ep);
	PResult Send(const char* data, int nBytes, int& flag);
	PResult Recv(char* dest, int nBytes, int& flag, int& bytesRecv);

	const SocketHandle getHandle() const { return m_handle; }
	IPVersion getIPVersion() { return m_ipversion; }
};

#endif
