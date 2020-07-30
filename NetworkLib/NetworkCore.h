#pragma once
#pragma comment(lib,"ws2_32")

#include <deque>
#include <queue>
#include <thread>
#include <mutex>
#include <WinSock2.h>

#include "Logger.h"
#include "PrimitiveTypes.h"
#include "Protocol.h"
#include "ErrorCode.h"

class ClientSessionManager;
class ClientSession;

class NetworkCore
{
protected:
	using UniquePtrThread = std::unique_ptr<std::thread>;


protected:
	ClientSessionManager* mClientSessionManager = nullptr;

	UniquePtrThread mSelectThread = nullptr;
	std::mutex mPacketMutex;
	std::mutex mSessionMutex;

	std::queue<Core::ReceivePacket> mReceivePacketQueue;

	SOCKET mAcceptSocket = INVALID_SOCKET;

	fd_set mReadSet{};
	fd_set mWriteSet{};

	bool mIsRunning = false;


public:
	NetworkCore() = default;
	~NetworkCore();


public:
	static void LoadConfig();


protected:
	Core::ErrorCode Bind();
	Core::ErrorCode Listen();
	Core::ErrorCode CheckSelectResult(int selectResult);
	Core::ErrorCode AcceptClient();
	Core::ErrorCode ReceiveClient(ClientSession& clientSession);
	Core::ErrorCode SendClient(ClientSession& clientSession);
	Core::ErrorCode SendProcess(ClientSession& clientSession, const uint16 packetId, const char* bodyData, const int bodySize);

	void PushReceivePacket(const Core::ReceivePacket receivePacket);
	void SelectProcess();
	void SelectClient(const fd_set& readSet, const fd_set& writeSet);
	void CloseSession(const Core::ErrorCode errorCode, const ClientSession& clientSession);


public:
	Core::ErrorCode Init(const int maxSessionSize);
	Core::ErrorCode Run();
	Core::ErrorCode Stop();

	Core::ReceivePacket GetReceivePacket();

	void Broadcast(const uint16 packetId, const char* bodyData, const int bodySize);

	Core::ErrorCode Send(const int32 sessionIndex, const uint16 packetId, const char* bodyData, const int bodySize);
	Core::ErrorCode Send(const uint64 sessionUniqueId, const uint16 packetId, const char* bodyData, const int bodySize);
};