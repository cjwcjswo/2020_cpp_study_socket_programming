#pragma once
#pragma comment(lib,"ws2_32")
#include <WinSock2.h>
#include <deque>
#include "ErrorCode.h"

class NetworkCore
{
private:
	ErrorCode Init();

private:
	SOCKET accept_socket_;
	std::deque<SOCKET> client_deque_;

public:
	NetworkCore();
	~NetworkCore();

	static void LoadConfig();

	ErrorCode Start();
	ErrorCode Stop();
};

