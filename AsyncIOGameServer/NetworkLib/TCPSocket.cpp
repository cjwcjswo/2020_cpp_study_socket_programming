#include "TCPSocket.h"

#include <WS2tcpip.h>
#include <mswsock.h>
#include "OverlappedIOContext.h"


using namespace NetworkLib;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode TCPSocket::ReuseAddr()
{
	// https://www.joinc.co.kr/w/Site/Network_Programing/AdvancedComm/SocketOption
	char reuseAddr = 1;
	if (setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) < 0)
	{
		return ErrorCode::SOCKET_INIT_REUSE_ADDR_FAIL;
	}

	return ErrorCode();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode TCPSocket::Create()
{
	mSocket = WSASocket
	(
		AF_INET, SOCK_STREAM, IPPROTO_TCP,
		nullptr, 0, WSA_FLAG_OVERLAPPED
	);
	if (mSocket == INVALID_SOCKET)
	{
		return ErrorCode::SOCKET_INIT_FAIL;
	}

	ErrorCode errorCode = ReuseAddr();
	if (errorCode != ErrorCode::SUCCESS)
	{
		return errorCode;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode TCPSocket::Bind(const wchar* ipAddress, const uint16 portNum)
{
	SOCKADDR_IN socketAddrIn;
	int socketAddrInSize = sizeof(socketAddrIn);
	ZeroMemory(&socketAddrIn, socketAddrInSize);
	socketAddrIn.sin_family = AF_INET;
	socketAddrIn.sin_port = htons(portNum);
	InetPton(AF_INET, ipAddress, &socketAddrIn.sin_addr);

	if (bind(mSocket, reinterpret_cast<SOCKADDR*>(&socketAddrIn), socketAddrInSize) == SOCKET_ERROR)
	{
		return ErrorCode::SOCKET_BIND_FAIL;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode TCPSocket::Listen(const int backlog)
{
	if (listen(mSocket, backlog) == SOCKET_ERROR)
	{
		return ErrorCode::SOCKET_LISTEN_FAIL;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode TCPSocket::AcceptAsync(TCPSocket* clientSocket)
{
	DWORD sockAddrSize = sizeof(SOCKADDR_IN) + 16;

	// TODO 최흥배. 아래 것도 pool로 관리하죠
	// 적용 완료
	OverlappedIOContext* context = GIOContextPool->Pop();
	if (context == nullptr)
	{
		return ErrorCode::IO_CONTEXT_ELEMENT_IS_NOT_ENOUGH;
	}
	context->mIOKey = IOKey::ACCEPT;

	BOOL isOK = AcceptEx
	(mSocket, clientSocket->mSocket, clientSocket->mAddressBuffer,
		0, sockAddrSize, sockAddrSize,
		nullptr, reinterpret_cast<LPOVERLAPPED>(context));
	if (!isOK)
	{
		int errorCode = WSAGetLastError();
		if (errorCode != WSA_IO_PENDING)
		{
			return ErrorCode::SOCKET_ACCEPT_ASYNC_FAIL;
		}
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void TCPSocket::Clear()
{
	mSocket = INVALID_SOCKET;
}
