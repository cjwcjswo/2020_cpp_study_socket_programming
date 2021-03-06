﻿#include "IOCPThread.h"

#include "Define.h"
#include "Logger.h"
#include "OverlappedIOContext.h"
#include "ClientSessionManager.h"
#include "ClientSession.h"
#include "TCPSocket.h"


using namespace NetworkLib;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI IOCPThread::IOCPSocketProcess()
{
	const int PENDING_COUNT = 100;
	OVERLAPPED_ENTRY entries[PENDING_COUNT];
	ULONG entryNum = 0;
	
	bool isOk = GetQueuedCompletionStatusEx
	(
		mIOCPHandle, entries, PENDING_COUNT, &entryNum, INFINITE, false
	);
	if (!isOk)
	{
		int errorCode = WSAGetLastError();
		if (errorCode != ERROR_ABANDONED_WAIT_0)
		{
			GLogger->PrintConsole(Color::RED, L"GQCS Failed: %d\n", errorCode);
		}

		return 0;
	}

	for (ULONG i = 0; i < entryNum; ++i)
	{
		OVERLAPPED_ENTRY entry = entries[i];
		DWORD transferred = entry.dwNumberOfBytesTransferred;
		OverlappedIOContext* ioContext = reinterpret_cast<OverlappedIOContext*>(entry.lpOverlapped);
		
		if (ioContext == nullptr)
		{
			GLogger->PrintConsole(Color::RED, L"io context is nullptr(entry: %d)\n", i);

			continue;
		}
		
		switch (ioContext->mIOKey)
		{
		case IOKey::ACCEPT:
		{
			std::lock_guard<std::mutex> lock(mSessionMutex);

			// TODO 최흥배
			// 애플리케이션에 새로운 연결이 발생했음을 알려줄 수 있어야 합니다.
			ErrorCode errorCode = mClientSessionManager->ConnectClientSession(mIOCPHandle, ioContext->mTCPSocket);
			if (errorCode != ErrorCode::SUCCESS)
			{
				GLogger->PrintConsole(Color::RED, L"Client %d Connect Fail\n", ioContext->mTCPSocket->mSocket);

				closesocket(ioContext->mTCPSocket->mSocket);
				ioContext->mTCPSocket->Clear();
				ioContext->mTCPSocket->Create();

				ErrorCode errorCode = mListenSocket->AcceptAsync(ioContext->mTCPSocket);
				if (errorCode != ErrorCode::SUCCESS)
				{
					GLogger->PrintConsole(Color::RED, L"Socket(index: %d) Accept Async Error: %d\n", i, static_cast<uint16>(errorCode));
				}

				break;
			}

			GLogger->PrintConsole(Color::LGREEN, L"New Client %d Connected!\n", ioContext->mTCPSocket->mSocket);

			break;
		}
		case IOKey::RECEIVE:
		{
			if (!ioContext->mSession->IsConnect())
			{
				GLogger->PrintConsole(Color::RED, L"%d Client Session Not Found(Receive)", ioContext->mSession->mTCPSocket->mSocket);
				break;
			}

			ClientSession* session = ioContext->mSession;
			if (entry.dwNumberOfBytesTransferred == 0)
			{
				// TODO 최흥배
				// 다른 스레드에서 send 작업이 걸려 있다면 아래는 스레드세이프하지 않습니다.
				// IOCP에서 Close 처리가 꽤 까다롭습니다
				// TODO: 일단 여러번 테스트해보고 문제 파악 & 고민해보고 수정하기
				session->Disconnect();

				// TODO 최흥배
				// 애플리케이션측에 연결이 끊어짐을 통보하지 못하고 있습니다
				GLogger->PrintConsole(Color::LGREEN, L"Client %d Disconnected!\n", session->mTCPSocket->mSocket);

				ErrorCode errorCode = mListenSocket->AcceptAsync(session->mTCPSocket);
				if (errorCode != ErrorCode::SUCCESS)
				{
					GLogger->PrintConsole(Color::RED, L"Socket(index: %d) Accept Async Error: %d\n", i, static_cast<uint16>(errorCode));
				}
				break;
			}

			session->ReceiveCompletion(transferred);

			PacketHeader* header;
			while (ioContext->mSession->mReceiveBuffer.DataSize() >= PACKET_HEADER_SIZE)
			{
				header = reinterpret_cast<PacketHeader*>(session->mReceiveBuffer.FrontData());
				uint16 requireBodySize = header->mPacketSize - PACKET_HEADER_SIZE;

				if (requireBodySize > 0)
				{
					if (requireBodySize > session->mReceiveBuffer.DataSize())
					{
						break;
					}
				}

				Packet receivePacket = { session->mIndexElement->mIndex, session->mUniqueId, header->mPacketId, 0, nullptr };
				if (requireBodySize > 0)
				{
					receivePacket.mBodyDataSize = requireBodySize;
					receivePacket.mBodyData = session->mReceiveBuffer.FrontData() + PACKET_HEADER_SIZE;
				}

				//TODO 최흥배
				// 뒤에 생각해봐야할 부분인데 패킷 처리쪽에서 처리에 비해 receive가 자주 발생하면(혹는 큰 데이터를 받거나) 덮어써여지는 문제가 발생할 수 있는데
				// 이 문제에 대해서 어떻게 처리해야 할지 고민해봐야겠습니다.
				// OK(채팅서버 이후 작업 예정)
				session->mReceiveBuffer.Pop(header->mPacketSize);

				mPushPacketFunction(receivePacket);
			}

			session->ReceiveAsync();

			break;
		}
		case IOKey::SEND:
		{
			// TODO 최흥배
			// 같은 세션 객체가 다른 스레드에서 Receive 이벤트에서 Close 처리를 하고 있는 중이거나 혹은 이미 막 한 상태에서 아래 코드가 호출되는 경우 문제가 없을까요?
			if (!ioContext->mSession->IsConnect())
			{
				GLogger->PrintConsole(Color::RED, L"%d Client Session Not Found(Receive)", ioContext->mSession->mTCPSocket->mSocket);
				break;
			}

			ioContext->mSession->SendCompletion(transferred);

			break;
		}
		}

		ioContext->Clear();
		GIOContextPool->Push(ioContext);
	}


	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void IOCPThread::Init(bool* isRunning, TCPSocket* listenSocket, const HANDLE iocpHandle, ClientSessionManager
	* clientSessionManager, std::function<void(const Packet)> pushPacketFunction)
{
	mIsRunning = isRunning;
	mListenSocket = listenSocket;
	mIOCPHandle = iocpHandle;
	mClientSessionManager = clientSessionManager;
	mPushPacketFunction = pushPacketFunction;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void IOCPThread::Join()
{
	mThread->join();
}
