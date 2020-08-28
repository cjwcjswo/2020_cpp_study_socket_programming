#include "TCPSocket.h"

#include <WS2tcpip.h>
#include <mswsock.h>
#include "OverlappedIOContext.h"


using namespace NetworkLib;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
TCPSocket::TCPSocket()
{
	
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void* TCPSocket::GetSocketExtensionAPI(SOCKET socket, GUID functionGUID)
{
	void* function = nullptr;
	GUID guid = functionGUID;
	DWORD bytes = 0;
	LONG result = WSAIoctl
	(
		socket,
		SIO_GET_EXTENSION_FUNCTION_POINTER,
		&guid, sizeof(guid),
		&function, sizeof(function),
		&bytes, nullptr, nullptr
	);
	if (result == SOCKET_ERROR)
	{
		return nullptr;
	}

	return function;
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
	LPFN_ACCEPTEX acceptEx = reinterpret_cast<LPFN_ACCEPTEX>(GetSocketExtensionAPI(mSocket, WSAID_ACCEPTEX));
	DWORD sockAddrSize = sizeof(SOCKADDR_IN) + 16;
	OverlappedIOAcceptContext* context = new OverlappedIOAcceptContext(clientSocket);

	BOOL isOK = acceptEx
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
