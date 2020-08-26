#pragma once

#include <WinSock2.h>


namespace NetworkLib
{
	class TCPSocket;

	enum class IOKey : ULONG_PTR
	{
		NONE = 0,
		ACCEPT = 1,
		RECEIVE = 2,
	};

	struct OverlappedIOContext : OVERLAPPED
	{
		TCPSocket* mTCPSocket = nullptr;

		OverlappedIOContext(TCPSocket* tcpSocket)
		{
			memset(this, 0, sizeof(*this));
			mTCPSocket = tcpSocket;
		}
	};
}


