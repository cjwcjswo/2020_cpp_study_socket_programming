#pragma once

#include <WinSock2.h>

#include "PrimitiveTypes.h"
#include "ErrorCode.h"

namespace NetworkLib
{
	class TCPSocket;

	class Network
	{
	private:
		TCPSocket* mListenSocket = nullptr;

		
	public:
		explicit Network() = default;
		~Network() = default;


	private:
		DWORD WINAPI IOCPSOcketReceiveProcess(PVOID param);


	public:
		ErrorCode Init();
		ErrorCode Run();

	};
}


