﻿#pragma once
#pragma comment(lib,"ws2_32")

#include <WinSock2.h>
#include <queue>
#include <mutex>
#include <thread>

#include "SList.h"
#include "Protocol.h"
#include "PrimitiveTypes.h"
#include "ErrorCode.h"


namespace NetworkLib
{
	struct OverlappedIOContext;
	class IOCPThread;
	class TCPSocket;
	class ClientSessionManager;
	class ClientSession;
	class Config;

	class Network
	{
	protected:
		TCPSocket* mListenSocket = nullptr;
		HANDLE mIOCPHandle = INVALID_HANDLE_VALUE;
		HANDLE mExitEvent = INVALID_HANDLE_VALUE;

		ClientSessionManager* mClientSessionManager = nullptr;

		IOCPThread* mIOCPThreads = nullptr;
		std::unique_ptr<std::thread> mSendThread = nullptr;

		std::queue<Packet> mReceivePacketQueue;
		std::mutex mReceivePacketMutex;

		Config* mConfig = nullptr;

		bool mIsRunning = false;


	public:
		Network() = default;
		~Network() = default;


	protected:
		void PushReceivePacket(const Packet receivePacket);


	public:
		ErrorCode Init();
		ErrorCode Run();
		void Stop();

		void SendRawData(const int32 sessionIndex, char* bodyData, const uint16 bodySize);

		void Send(const int32 sessionIndex, const uint16 packetId, char* bodyData, const uint16 bodySize);

		Packet GetReceivePacket();
	};
}


