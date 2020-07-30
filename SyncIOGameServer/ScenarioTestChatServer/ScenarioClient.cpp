#include "ScenarioClient.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ScenarioClient::Init()
{
	WSADATA wsaData;

	if (SOCKET_ERROR == WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		return ErrorCode::WSA_START_UP_FAIL;
	}

	mConnectSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == mConnectSocket)
	{
		return ErrorCode::SOCKET_INIT_FAIL;
	}

	GLogger->PrintConsole(Color::GREEN, L"Scenario Client Init Success");

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ScenarioClient::Connect(const wchar* address, const uint16 portNum)
{
	SOCKADDR_IN socketAddrIn;
	int socketAddrInSize = sizeof(socketAddrIn);
	ZeroMemory(&socketAddrIn, socketAddrInSize);
	socketAddrIn.sin_family = AF_INET;
	socketAddrIn.sin_port = htons(32452);
	InetPton(AF_INET, address, &socketAddrIn.sin_addr);

	if (SOCKET_ERROR == connect(mConnectSocket, reinterpret_cast<sockaddr*>(&socketAddrIn), socketAddrInSize))
	{
		return ErrorCode::SOCKET_CONNECT_FAIL;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ScenarioClient::Disconnect()
{
	if (SOCKET_ERROR == closesocket(mConnectSocket))
	{
		return ErrorCode::SOCKET_DISCONNECT_FAIL;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ScenarioClient::Send()
{
	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ScenarioClient::Receive()
{
	return ErrorCode::SUCCESS;
}