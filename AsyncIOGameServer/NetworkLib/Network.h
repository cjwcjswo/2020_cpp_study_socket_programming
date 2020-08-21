#pragma once

#include <WinSock2.h>
#include <thread>
#include <queue>

#include "PrimitiveTypes.h"
#include "ErrorCode.h"


namespace NetworkLib
{
	class TCPSocket;
	class ClientSessionManager;
	class ClientSession;

	enum class IOKey : ULONG_PTR
	{
		NONE = 0,
		ACCEPT = 1,
	};

	class Network
	{
	protected:
		using UniquePtrThread = std::unique_ptr<std::thread>;

		
	protected:
		TCPSocket* mListenSocket = nullptr;
		HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;
		CRITICAL_SECTION mCriticalSection;

		ClientSessionManager* mClientSessionManager = nullptr;

		UniquePtrThread mSocketProcessThread;

		int mMaxClientNum = 0;

		
	public:
		explicit Network() = default;
		~Network() = default;


	protected:
		DWORD WINAPI IOCPSocketProcess();


	public:
		ErrorCode Init(int maxClientNum);
		ErrorCode Run();

	};
}


