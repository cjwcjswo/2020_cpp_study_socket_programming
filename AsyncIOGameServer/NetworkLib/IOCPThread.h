#pragma once

#include <WinSock2.h>
#include <thread>
#include <mutex>

namespace NetworkLib
{
	class ClientSessionManager;

	class IOCPThread
	{
	protected:
		using UniquePtrThread = std::unique_ptr<std::thread>;


	protected:
		UniquePtrThread mThread = nullptr;
		HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;
		ClientSessionManager* mClientSessionManager = nullptr;
		std::mutex mSessionMutex;


	public:
		IOCPThread() = default;
		~IOCPThread() = default;


	protected:
		DWORD WINAPI IOCPSocketProcess();


	public:
		void Init(const HANDLE iocpHandle, ClientSessionManager* clientSessionManager);
		void Run();
	};
}

