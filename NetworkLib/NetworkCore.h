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
private:
	using UniquePtrThread = std::unique_ptr<std::thread>;


private:
	ClientSessionManager* mClientSessionManager;

	UniquePtrThread mSelectThread = nullptr;
	std::mutex mMutex = {};

	std::queue<Core::ReceivePacket> mReceivePacketQueue{};

	SOCKET mAcceptSocket = INVALID_SOCKET;

	fd_set mReadSet{};
	fd_set mWriteSet{};

	bool mIsRunning = false;


public:
	NetworkCore();
	~NetworkCore();


public:
	static void LoadConfig();


private:
	Core::ErrorCode Bind();
	Core::ErrorCode Listen();
	Core::ErrorCode CheckSelectResult(int selectResult);
	Core::ErrorCode AcceptClient();
	Core::ErrorCode ReceiveClient(ClientSession& clientSession, const fd_set& readSet);
	Core::ErrorCode SendClient(ClientSession& clientSession, const fd_set& readSet);
	
	void PushReceivePacket(const Core::ReceivePacket receivePacket);
	void SelectProcess();
	void SelectClient(const fd_set& readSet, const fd_set& writeSet);
	void CloseSession(const Core::ErrorCode errorCode, const ClientSession& clientSession);


public:
	Core::ErrorCode Init();
	Core::ErrorCode Run();
	Core::ErrorCode Stop();
	Core::ReceivePacket GetReceivePacket();
};

