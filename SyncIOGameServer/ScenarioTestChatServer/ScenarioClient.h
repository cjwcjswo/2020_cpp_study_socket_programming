#pragma once
#pragma comment(lib,"ws2_32")

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "../NetworkLib/Protocol.h"
#include "../NetworkLib/Logger.h"
#include "../ChatServer/Protocol.h"
#include "ErrorCode.h"

class ScenarioClient
{
private:
	constexpr static int BUFFER_SIZE = 1024;


private:
	SOCKET mConnectSocket;
	char mSendBuffer[BUFFER_SIZE];


public:
	ScenarioClient() = default;
	~ScenarioClient() = default;

	ErrorCode Init();

	ErrorCode Connect(const wchar* address, const uint16 portNum);
	ErrorCode Disconnect();
	ErrorCode Send(const uint16 packetId, const char* bodyData, const int bodySize);
	ErrorCode Receive();
};

