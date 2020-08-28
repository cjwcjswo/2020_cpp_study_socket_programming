#pragma once
#pragma comment(lib,"ws2_32")

#include <WinSock2.h>
#include <queue>
#include <mutex>
#include "Protocol.h"
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

		std::queue<Packet> mReceivePacketQueue;
		std::mutex mReceivePacketMutex;

		int mMaxClientNum = 0;
		int mMaxThreadNum = 10;

		
	public:
		Network() = default;
		~Network() = default;


	protected:
		void PushReceivePacket(const Packet receivePacket);


	public:
		ErrorCode Init(int maxClientNum);
		ErrorCode Run();

		void SendRawData(const int32 sessionIndex, char* bodyData, const uint16 bodySize);

		void Send(const int32 sessionIndex, const uint16 packetId, char* bodyData, const uint16 bodySize);
		void Send(const uint64 sessionUniqueId, const uint16 packetId, char* bodyData, const uint16 bodySize);

		Packet GetReceivePacket();

	};
}


