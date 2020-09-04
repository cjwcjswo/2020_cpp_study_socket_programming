#pragma once

#include <WinSock2.h>

#include "SList.h"


namespace NetworkLib
{
	class TCPSocket;
	class ClientSession;

	enum class IOKey : ULONG_PTR
	{
		NONE = 0,
		ACCEPT = 1,
		RECEIVE = 2,
		SEND = 3,
	};

	
	struct OverlappedIOContext : OVERLAPPED
	{
		IOKey mIOKey = IOKey::NONE;
		TCPSocket* mTCPSocket = nullptr;
		ClientSession* mSession = nullptr;
		WSABUF mWSABuf;

		OverlappedIOContext()
		{
			Clear();
		}

		void Clear()
		{
			memset(this, 0, sizeof(*this));
		}
	};

	 inline SList<OverlappedIOContext>* GIOContextPool = new SList<OverlappedIOContext>;
}