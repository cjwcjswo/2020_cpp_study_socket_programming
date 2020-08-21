#pragma once
#pragma comment(lib,"ws2_32")

#include <WinSock2.h>

#include "ErrorCode.h"
#include "PrimitiveTypes.h"

namespace NetworkLib
{
	class TCPSocket
	{
	protected:
		char mBuffer[512] = { 0, };
		char mAddressBuffer[512] = { 0, };


	public:
		SOCKET mSocket = INVALID_SOCKET;


	public:
		explicit TCPSocket();
		~TCPSocket() = default;


	protected:
		void* GetSocketExtensionAPI(SOCKET socket, GUID functionGUID);

		ErrorCode ReuseAddr();


	public:
		ErrorCode Create();
		ErrorCode Bind(const wchar* ipAddress, const uint16 portNum);
		ErrorCode Listen(const int backlog = SOMAXCONN);
		ErrorCode AcceptAsync(TCPSocket* clientSocket);

		void Clear();
	};

	struct SocketItem : OVERLAPPED
	{
		TCPSocket* mTCPSocket = nullptr;
		
		SocketItem(TCPSocket* tcpSocket)
		{
			memset(this, 0, sizeof(*this));
			mTCPSocket = tcpSocket;
		}
	};
}
