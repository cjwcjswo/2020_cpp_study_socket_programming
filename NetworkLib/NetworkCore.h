#pragma once
#pragma comment(lib,"ws2_32")
#include <deque>
#include <queue>
#include <thread>
#include <mutex>
#include <WinSock2.h>

#include "ClientSessionManager.h"
#include "Protocol.h"
#include "ErrorCode.h"

using UniquePtrThread = std::unique_ptr<std::thread>;

class NetworkCore
{
private:
	ErrorCode Init();
	ErrorCode Bind();
	ErrorCode Listen();
	ErrorCode CheckSelectResult(int select_result);
	ErrorCode AcceptClient();
	ErrorCode ReceiveClient(SharedPtrClientSession client_session, const fd_set& read_set);
	ErrorCode SendClient(SharedPtrClientSession client_session, const fd_set& read_set);

	void PushReceivePacket(const ReceivePacket receive_packet);
	void SelectProcess();
	void SelectClient(const fd_set& read_set, const fd_set& write_set);
	void CloseSession(const ErrorCode error_code, const SharedPtrClientSession client_session);

private:
	ClientSessionManager client_session_manager{ 50 };
	std::queue<ReceivePacket> receive_packet_queue_{};
	UniquePtrThread ptr_select_thread = nullptr;
	std::mutex mutex_ = {};

	SOCKET accept_socket_ = INVALID_SOCKET;

	fd_set read_set_{};
	fd_set write_set_{};

	bool is_running_ = false;

public:
	NetworkCore();
	~NetworkCore();

	static void LoadConfig();

	ErrorCode Run();
	ErrorCode Stop();
	ReceivePacket GetReceivePacket();
};

