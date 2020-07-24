#pragma once
#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <deque>
#include "ErrorCode.h"

class NetworkCore
{
private:
	ErrorCode Init();
	ErrorCode Bind();
	ErrorCode Listen();
	ErrorCode CheckSelectResult(int select_result);
	ErrorCode AcceptClient();
	ErrorCode SelectClient(const fd_set& read_set, const fd_set& write_set);

private:
	SOCKET accept_socket_;
	std::deque<SOCKET> client_deque_;

	fd_set read_set_;
	fd_set write_set_;

public:
	NetworkCore();
	~NetworkCore();

	static void LoadConfig();

	ErrorCode Run();
	ErrorCode Stop();
};

