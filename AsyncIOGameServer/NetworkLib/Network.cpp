#include "Network.h"
#include "OverlappedIOContext.h"
#include "IOCPThread.h"
#include "TCPSocket.h"
#include "Logger.h"
#include "ClientSessionManager.h"
#include "ClientSession.h"


using namespace NetworkLib;


ErrorCode NetworkLib::Network::Init(int maxClientNum)
{
	mMaxClientNum = maxClientNum;
	
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

	mIOCPThreads = new IOCPThread[mMaxThreadNum];

	return ErrorCode::SUCCESS;
}

ErrorCode Network::Run()
{
	HANDLE exitEvent = CreateEvent(nullptr, true, false, nullptr);

	GLogger->PrintConsole(Color::LBLUE, "Run Server~~~~~\n");

	mIOCPHandle = CreateIoCompletionPort(reinterpret_cast<HANDLE>(mListenSocket->mSocket), nullptr, static_cast<ULONG_PTR>(IOKey::ACCEPT), 0);

	for (int i = 0; i < mMaxThreadNum; ++i)
	{
		mIOCPThreads[i].Init(mIOCPHandle, mClientSessionManager);
		mIOCPThreads[i].Run();
	}

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
