#pragma once
#pragma comment(lib,"ws2_32")

#include <deque>
#include <queue>
#include <vector>
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
	class Config;

	class Network
	{
	protected:
		using UniquePtrThread = std::unique_ptr<std::thread>;


	protected:
		Config* mConfig;

		ClientSessionManager* mClientSessionManager = nullptr;
		UniquePtrThread mSelectThread = nullptr;
		UniquePtrThread mSendThread = nullptr;
		TCPSocket* mAcceptSocket = nullptr;

		std::mutex mReceivePacketMutex;
		std::mutex mSendPacketMutex;
		std::mutex mSessionMutex;

		std::queue<NetworkLib::Packet> mReceivePacketQueue;
		std::queue<NetworkLib::Packet> mSendPacketQueue;

		fd_set mReadSet{};
		fd_set mWriteSet{};

		bool mIsRunning = false;


	public:
		Network() = default;
		~Network();


	protected:
		NetworkLib::ErrorCode CheckSelectResult(int selectResult);
		NetworkLib::ErrorCode AcceptClient();
		NetworkLib::ErrorCode ReceiveClient(ClientSession& clientSession);
		NetworkLib::ErrorCode SendClient(ClientSession& clientSession);

		void SendProcess();

		void PushReceivePacket(const NetworkLib::Packet receivePacket);
		void SelectProcess();
		void SelectClient(const fd_set& readSet);
		void CloseSession(const NetworkLib::ErrorCode errorCode, ClientSession& clientSession);


	public:
		NetworkLib::ErrorCode Init();
		NetworkLib::ErrorCode Run();
		NetworkLib::ErrorCode Stop();

		NetworkLib::Packet GetReceivePacket();

		void Broadcast(const uint16 packetId, char* bodyData, const uint16 bodySize);
		void Broadcast(const uint16 packetId, char* bodyData, const uint16 bodySize, const int exceptUserCount, ...);

		void Send(const int32 sessionIndex, const uint16 packetId, char* bodyData, const uint16 bodySize);
		void Send(const uint64 sessionUniqueId, const uint16 packetId, char* bodyData, const uint16 bodySize);
	};
}