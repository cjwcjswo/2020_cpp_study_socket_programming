#pragma once
#pragma comment(lib,"ws2_32")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "../../NetworkLib/Logger.h"
#include "ErrorCode.h"

class ScenarioClient
{
private:
	SOCKET mConnectSocket;

public:
	ScenarioClient() = default;
	~ScenarioClient() = default;

	ErrorCode Init();

	ErrorCode Connect(const wchar* address, const uint16 portNum);
	ErrorCode Disconnect();
	ErrorCode Send();
	ErrorCode Receive();
};

