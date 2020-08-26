#pragma once

#include <WinSock2.h>
#include <queue>

#include "PrimitiveTypes.h"
#include "ErrorCode.h"


namespace NetworkLib
{
	class IOCPThread;
	class TCPSocket;
	class ClientSessionManager;
	class ClientSession;

	class Network
	{
	protected:
		TCPSocket* mListenSocket = nullptr;
		HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;

		ClientSessionManager* mClientSessionManager = nullptr;

		IOCPThread* mIOCPThreads = nullptr;

		int mMaxClientNum = 0;
		int mMaxThreadNum = 10;

		
	public:
		Network() = default;
		~Network() = default;


	public:
		ErrorCode Init(int maxClientNum);
		ErrorCode Run();

	};
}


