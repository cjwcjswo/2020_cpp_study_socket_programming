#include "Network.h"
#include "TCPSocket.h"
#include "Logger.h"


using namespace NetworkLib;


DWORD WINAPI Network::IOCPSOcketReceiveProcess(PVOID param)
{
	SocketItem* socketItem = nullptr;
	DWORD trBytes = 0;
	ULONG_PTR devKey = 0;

	while (true)
	{
		/*GetQueuedCompletionStatus
		(

		);*/

	}

	return 0;
}

ErrorCode NetworkLib::Network::Init()
{
	mListenSocket = new TCPSocket();

	ErrorCode errorCode = mListenSocket->Create();
	if (errorCode != ErrorCode::SUCCESS)
	{
		GLogger->PrintConsole(Color::RED, L"Listen Socket Create Error: %d", static_cast<uint16>(errorCode));
		return errorCode;
	}

	errorCode = mListenSocket->Bind(L"127.0.0.1", 32452);
	if (errorCode != ErrorCode::SUCCESS)
	{
		GLogger->PrintConsole(Color::RED, L"Listen Socket Bind Error: %d", static_cast<uint16>(errorCode));
		return errorCode;
	}

	errorCode = mListenSocket->Listen();
	if (errorCode != ErrorCode::SUCCESS)
	{
		GLogger->PrintConsole(Color::RED, L"Listen Socket Listen Error: %d", static_cast<uint16>(errorCode));
		return errorCode;
	}

	return ErrorCode::SUCCESS;
}

ErrorCode NetworkLib::Network::Run()
{
	return ErrorCode::SUCCESS;
}
