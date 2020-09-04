#pragma once

#include <WinSock2.h>

#include "ErrorCode.h"
#include "PrimitiveTypes.h"


namespace NetworkLib
{
	class TCPSocket
	{
	protected:
		char* mAddressBuffer = nullptr;


	public:
		SOCKET mSocket = INVALID_SOCKET;


	public:
		explicit TCPSocket(uint32 socketAddressBufferSize) { mAddressBuffer = new char[socketAddressBufferSize]; };
		~TCPSocket() = default;


	protected:
		ErrorCode ReuseAddr();


	public:
		ErrorCode Create();
		ErrorCode Bind(const wchar* ipAddress, const uint16 portNum);
		ErrorCode Listen(const int backlog = SOMAXCONN);

		ErrorCode AcceptAsync(TCPSocket* clientSocket);

		void Clear();
	};
}

