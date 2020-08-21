#include "Network.h"
#include "TCPSocket.h"
#include "Logger.h"
#include "ClientSessionManager.h"
#include "ClientSession.h"


using namespace NetworkLib;


DWORD WINAPI Network::IOCPSocketProcess()
{
	SocketItem* socketItem = nullptr;
	DWORD trBytes = 0;
	IOKey devKey = IOKey::NONE;
	
	while (true)
	{
		// Blcking
		bool isOk = GetQueuedCompletionStatus
		(
			mIOCPHandle, &trBytes, reinterpret_cast<PULONG_PTR>(&devKey), reinterpret_cast<LPOVERLAPPED*>(&socketItem), INFINITE
		);
		if (!isOk)
		{
			if (socketItem != nullptr)
			{
				// 소켓 관련 에러
				continue;
			}

			int errorCode = WSAGetLastError();
			if (errorCode != ERROR_ABANDONED_WAIT_0)
			{
				GLogger->PrintConsole(Color::RED, L"GQCS Failed: %d\n", errorCode);
			}

			break;
		}

		switch (devKey)
		{
		case IOKey::ACCEPT:
		{
			GLogger->PrintConsole(Color::LGREEN, L"New Client %d Connected!\n", socketItem->mTCPSocket->mSocket);

			CreateIoCompletionPort((HANDLE)socketItem->mTCPSocket->mSocket, mIOCPHandle, static_cast<ULONG_PTR>(IOKey::ACCEPT), 0);

			break;
		}
		//case IOKey::CLIENT_CONNECT:
		//{
		//	CreateIoCompletionPort((HANDLE)socketItem->mTCPSocket->mSocket, mIOCPHandle, static_cast<ULONG_PTR>(IOKey::ACCEPT), 0);

		//	GLogger->PrintConsole(Color::LGREEN, L"New Client %d Connected!\n", socketItem->mTCPSocket->mSocket);

		//	// Insert Client Socket
		//}

		}
		
	}

	return 0;
}

ErrorCode NetworkLib::Network::Init(int maxClientNum)
{
	mMaxClientNum = maxClientNum;

	InitializeCriticalSection(&mCriticalSection);

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR)
	{
		return ErrorCode::WSA_START_UP_FAIL;
	}

	mListenSocket = new TCPSocket();

	ErrorCode errorCode = mListenSocket->Create();
	if (errorCode != ErrorCode::SUCCESS)
	{
		GLogger->PrintConsole(Color::RED, L"Listen Socket Create Error: %d\n", static_cast<uint16>(errorCode));
		return errorCode;
	}

	errorCode = mListenSocket->Bind(L"127.0.0.1", 32452);
	if (errorCode != ErrorCode::SUCCESS)
	{
		GLogger->PrintConsole(Color::RED, L"Listen Socket Bind Error: %d\n", static_cast<uint16>(errorCode));
		return errorCode;
	}

	errorCode = mListenSocket->Listen();
	if (errorCode != ErrorCode::SUCCESS)
	{
		GLogger->PrintConsole(Color::RED, L"Listen Socket Listen Error: %d\n", static_cast<uint16>(errorCode));
		return errorCode;
	}

	mClientSessionManager = new ClientSessionManager();
	errorCode = mClientSessionManager->Init(mMaxClientNum, 1024);
	if (errorCode != ErrorCode::SUCCESS)
	{
		GLogger->PrintConsole(Color::RED, L"ClientSessionManager Init Error: %d\n", static_cast<uint16>(errorCode));
		return errorCode;
	}

	return ErrorCode::SUCCESS;
}

ErrorCode NetworkLib::Network::Run()
{
	HANDLE exitEvent = CreateEvent(nullptr, true, false, nullptr);

	GLogger->PrintConsole(Color::LBLUE, "Run Server~~~~~\n");

	mIOCPHandle = CreateIoCompletionPort(reinterpret_cast<HANDLE>(mListenSocket->mSocket), nullptr, static_cast<ULONG_PTR>(IOKey::ACCEPT), 0);

	mSocketProcessThread = std::make_unique<std::thread>([&]() {IOCPSocketProcess(); });

	// Reserve Accept Async
	ErrorCode errorCode;
	for (int i = 0; i < mMaxClientNum; ++i)
	{
		ClientSession* session = mClientSessionManager->FindClientSession(i);
		if (session == nullptr)
		{
			GLogger->PrintConsole(Color::RED, L"ClientSession(index: %d) Not Exist Error: %d\n", i, static_cast<uint16>(ErrorCode::CLIENT_SESSION_NOT_EXIST));

			return ErrorCode::CLIENT_SESSION_NOT_EXIST;
		}

		errorCode = mListenSocket->AcceptAsync(session->mTCPSocket);
		if (errorCode != ErrorCode::SUCCESS)
		{
			GLogger->PrintConsole(Color::RED, L"Socket(index: %d) Accept Async Error: %d\n", i, static_cast<uint16>(errorCode));

			return errorCode;
		}
	}

	WaitForSingleObject(exitEvent, INFINITE);

	GLogger->PrintConsole(Color::LBLUE, "Stop Server~~~~~~\n");

	return ErrorCode::SUCCESS;
}
