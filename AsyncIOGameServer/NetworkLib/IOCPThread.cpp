#include "IOCPThread.h"
#include "Define.h"
#include "Logger.h"
#include "OverlappedIOContext.h"
#include "ClientSessionManager.h"
#include "ClientSession.h"
#include "TCPSocket.h"


using namespace NetworkLib;


DWORD WINAPI IOCPThread::IOCPSocketProcess()
{
	OverlappedIOContext* ioContext = nullptr;
	DWORD trBytes = 0;
	IOKey devKey = IOKey::NONE;

	while (true)
	{
		// Blcking
		bool isOk = GetQueuedCompletionStatus
		(
			mIOCPHandle, &trBytes, reinterpret_cast<PULONG_PTR>(&devKey), reinterpret_cast<LPOVERLAPPED*>(&ioContext), INFINITE
		);
		if (!isOk)
		{
			if (ioContext != nullptr)
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
			std::lock_guard<std::mutex> lock(mSessionMutex);

			int32 sessionIndex = mClientSessionManager->AllocClientSessionIndex();
			if (sessionIndex == INVALID_INDEX)
			{
				GLogger->PrintConsole(Color::RED, L"%d AllocClientSessionIndex Fail", ioContext->mTCPSocket->mSocket);
				continue;
			}

			ClientSession session{ sessionIndex, mClientSessionManager->GenerateUniqueId(), ioContext->mTCPSocket };
			mClientSessionManager->ConnectClientSession(session);

			CreateIoCompletionPort((HANDLE)ioContext->mTCPSocket->mSocket, mIOCPHandle, static_cast<ULONG_PTR>(IOKey::RECEIVE), 0);

			GLogger->PrintConsole(Color::LGREEN, L"New Client %d Connected!\n", ioContext->mTCPSocket->mSocket);

			break;
		}
		case IOKey::RECEIVE:
		{

			break;
		}
		}

	}

	return 0;
}

void IOCPThread::Init(const HANDLE iocpHandle, ClientSessionManager* clientSessionManager)
{
	mIOCPHandle = iocpHandle;
	mClientSessionManager = clientSessionManager;
}

void IOCPThread::Run()
{
	mThread = std::make_unique<std::thread>(
		[&]()
		{
			while (true)
			{
				IOCPSocketProcess();
			}
		}
	);
}
