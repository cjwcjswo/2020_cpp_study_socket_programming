#include <WS2tcpip.h>

#include "TCPSocket.h"


using namespace Core;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode TCPSocket::SetLingerMode()
{
	linger ling = { 0, 0 };
	if (setsockopt(mSocket, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(ling)) < 0)
	{
		return ErrorCode::SOCKET_SET_OPTION_FAIL;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode TCPSocket::SetNonBlockingMode()
{
	// Set Non-Blocking Mode (https://www.joinc.co.kr/w/man/4200/ioctlsocket)
	unsigned long mode = 1;
	if (SOCKET_ERROR == ioctlsocket(mSocket, FIONBIO, &mode))
	{
		return ErrorCode::SOCKET_SET_FIONBIO_FAIL;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode TCPSocket::ReuseAddr()
{
	// https://www.joinc.co.kr/w/Site/Network_Programing/AdvancedComm/SocketOption
	char reuseAddr = 1;
	if (setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) < 0)
	{
		return ErrorCode::SOCKET_INIT_REUSE_ADDR_FAIL;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode TCPSocket::Create()
{
	mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == mSocket)
	{
		return ErrorCode::SOCKET_INIT_FAIL;
	}

	ReuseAddr();

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode TCPSocket::Bind(const wchar* address, const uint16 portNum)
{
	SOCKADDR_IN socketAddrIn;
	int socketAddrInSize = sizeof(socketAddrIn);
	ZeroMemory(&socketAddrIn, socketAddrInSize);
	socketAddrIn.sin_family = AF_INET;
	socketAddrIn.sin_port = htons(portNum);
	InetPton(AF_INET, address, &socketAddrIn.sin_addr);

	if (SOCKET_ERROR == bind(mSocket, reinterpret_cast<SOCKADDR*>(&socketAddrIn), socketAddrInSize))
	{
		return ErrorCode::SOCKET_BIND_FAIL;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode TCPSocket::Listen()
{
	if (SOCKET_ERROR == listen(mSocket, SOMAXCONN))
	{
		return ErrorCode::SOCKET_LISTEN_FAIL;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TCPSocket::Send(char* sendBuffer, const int bufferSize)
{
	return send(mSocket, sendBuffer, bufferSize, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int TCPSocket::Receive(char* receiveBuffer, const int bufferSize)
{
	return recv(mSocket, receiveBuffer, bufferSize, 0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TCPSocket TCPSocket::Accept()
{
	SOCKADDR_IN socketAddrIn;
	int size = sizeof(socketAddrIn);
	TCPSocket clientSocket(accept(mSocket, (SOCKADDR*)&socketAddrIn, &size));

	return clientSocket;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TCPSocket::Close()
{
	if (INVALID_SOCKET == mSocket)
	{
		return;
	}
	closesocket(mSocket);
	Clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TCPSocket::Clear()
{
	mSocket = INVALID_SOCKET;
}