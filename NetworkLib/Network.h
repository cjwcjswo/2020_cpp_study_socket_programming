#pragma once
#pragma comment(lib,"ws2_32")

#include <deque>
#include <queue>
#include <thread>
#include <mutex>
#include <WinSock2.h>

#include "PrimitiveTypes.h"
#include "Protocol.h"
#include "ErrorCode.h"


class TCPSocket;
class ClientSessionManager;
class ClientSession;

namespace NetworkLib
{
	class Network
	{
	protected:
		using UniquePtrThread = std::unique_ptr<std::thread>;


	protected:
		ClientSessionManager* mClientSessionManager = nullptr;
		UniquePtrThread mSelectThread = nullptr;
		TCPSocket* mAcceptSocket = nullptr;

		std::mutex mPacketMutex;
		std::mutex mSessionMutex;

		std::queue<NetworkLib::ReceivePacket> mReceivePacketQueue;

		fd_set mReadSet{};
		fd_set mWriteSet{};

		bool mIsRunning = false;


	public:
		Network() = default;
		~Network();


	public:
		static void LoadConfig();


	protected:
		NetworkLib::ErrorCode CheckSelectResult(int selectResult);
		NetworkLib::ErrorCode AcceptClient();
		NetworkLib::ErrorCode ReceiveClient(ClientSession& clientSession);
		NetworkLib::ErrorCode SendClient(ClientSession& clientSession);
		NetworkLib::ErrorCode SendProcess(ClientSession& clientSession, const uint16 packetId, const char* bodyData, const int bodySize);

		void PushReceivePacket(const NetworkLib::ReceivePacket receivePacket);
		void SelectProcess();
		void SelectClient(const fd_set& readSet, const fd_set& writeSet);
		void CloseSession(const NetworkLib::ErrorCode errorCode, ClientSession& clientSession);


	public:
		NetworkLib::ErrorCode Init(const int maxSessionSize);
		NetworkLib::ErrorCode Run();
		NetworkLib::ErrorCode Stop();

		NetworkLib::ReceivePacket GetReceivePacket();

		void Broadcast(const uint16 packetId, const char* bodyData, const int bodySize);

		NetworkLib::ErrorCode Send(const int32 sessionIndex, const uint16 packetId, const char* bodyData, const int bodySize);
		NetworkLib::ErrorCode Send(const uint64 sessionUniqueId, const uint16 packetId, const char* bodyData, const int bodySize);
	};
}