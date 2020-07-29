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

class ClientSessionManager;
class ClientSession;


class NetworkCore
{
private:
	using UniquePtrClientSessionManager = std::unique_ptr<ClientSessionManager>;
	using UniquePtrThread = std::unique_ptr<std::thread>;
	using SharedPtrClientSession = std::shared_ptr<ClientSession>;


private:
	UniquePtrClientSessionManager mClientSessionManager = nullptr;

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
	ErrorCode Init();
	ErrorCode Bind();
	ErrorCode Listen();
	ErrorCode CheckSelectResult(int selectResult);
	ErrorCode AcceptClient();
	ErrorCode ReceiveClient(SharedPtrClientSession clientSession, const fd_set& readSet);
	ErrorCode SendClient(SharedPtrClientSession clientSession, const fd_set& readSet);
	
	void PushReceivePacket(const Core::ReceivePacket receivePacket);
	void SelectProcess();
	void SelectClient(const fd_set& readSet, const fd_set& writeSet);
	void CloseSession(const ErrorCode errorCode, const SharedPtrClientSession clientSession);


public:
	ErrorCode Run();
	ErrorCode Stop();
	Core::ReceivePacket GetReceivePacket();
};

