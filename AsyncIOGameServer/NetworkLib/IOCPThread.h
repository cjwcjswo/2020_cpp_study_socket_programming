#pragma once

#include <WinSock2.h>
#include <thread>
#include <mutex>
#include <functional>
#include "Protocol.h"


namespace NetworkLib
{
	class ClientSessionManager;
	class TCPSocket;

	class IOCPThread
	{
	protected:
		using UniquePtrThread = std::unique_ptr<std::thread>;


	protected:
		UniquePtrThread mThread = nullptr;
		HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;
		TCPSocket* mListenSocket = nullptr;

		ClientSessionManager* mClientSessionManager = nullptr;
		std::mutex mSessionMutex;
		std::function<void(const Packet)> mPushPacketFunction;

		bool* mIsRunning = nullptr;


	public:
		IOCPThread() = default;
		~IOCPThread() = default;


	protected:
		DWORD WINAPI IOCPSocketProcess();


	public:
		void Init(bool* isRunning, TCPSocket* listenSocket, const HANDLE iocpHandle, ClientSessionManager* clientSessionManager, std::function<void(const Packet)> pushPacketFunction);
		void Run();
		void Join();
	};
}

