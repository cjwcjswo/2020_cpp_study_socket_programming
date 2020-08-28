#include "IOCPThread.h"

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
	OverlappedIOContext* ioContext = nullptr;
	DWORD transferred = 0;
	IOKey ioKey = IOKey::NONE;

	bool isOk = GetQueuedCompletionStatus
	(
		mIOCPHandle, &transferred, reinterpret_cast<PULONG_PTR>(&ioKey), reinterpret_cast<LPOVERLAPPED*>(&ioContext), INFINITE
	);
	if (!isOk)
	{
		if (ioContext != nullptr)
		{
			// 소켓 관련 에러
			return 0;
		}

		int errorCode = WSAGetLastError();
		if (errorCode != ERROR_ABANDONED_WAIT_0)
		{
			GLogger->PrintConsole(Color::RED, L"GQCS Failed: %d\n", errorCode);
		}

		return 0;
	}

	switch (ioContext->mIOKey)
	{
	case IOKey::ACCEPT:
	{
		std::lock_guard<std::mutex> lock(mSessionMutex);

		int32 sessionIndex = mClientSessionManager->AllocClientSessionIndex();
		if (sessionIndex == INVALID_INDEX)
		{
			GLogger->PrintConsole(Color::RED, L"%d AllocClientSessionIndex Fail", ioContext->mTCPSocket->mSocket);
			break;
		}

		ClientSession session{ sessionIndex, mClientSessionManager->GenerateUniqueId(), ioContext->mTCPSocket };
		ClientSession& newSession = mClientSessionManager->ConnectClientSession(session);

		CreateIoCompletionPort((HANDLE)ioContext->mTCPSocket->mSocket, mIOCPHandle, static_cast<ULONG_PTR>(IOKey::RECEIVE), 0);
		newSession.ReceiveAsync();

		GLogger->PrintConsole(Color::LGREEN, L"New Client %d Connected!\n", ioContext->mTCPSocket->mSocket);

		break;
	}
	case IOKey::RECEIVE:
	{
		ClientSession* session = mClientSessionManager->FindClientSessionBySocket(ioContext->mTCPSocket->mSocket);
		if (session == nullptr)
		{
			GLogger->PrintConsole(Color::RED, L"%d Client Session Not Found(Receive)", ioContext->mTCPSocket->mSocket);
			break;
		}

		session->ReceiveCompletion(transferred);

		PacketHeader* header;
		while (session->mReceiveBuffer.DataSize() >= PACKET_HEADER_SIZE)
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

			Packet receivePacket = { session->mIndex, session->mUniqueId, header->mPacketId, 0, nullptr };
			if (requireBodySize > 0)
			{
				receivePacket.mBodyDataSize = requireBodySize;
				receivePacket.mBodyData = session->mReceiveBuffer.FrontData() + PACKET_HEADER_SIZE;
			}

			session->mReceiveBuffer.Pop(header->mPacketSize);

			mPushPacketFunction(receivePacket);
		}

		session->ReceiveAsync();

		break;
	}
	case IOKey::SEND:
	{
		ClientSession* session = mClientSessionManager->FindClientSessionBySocket(ioContext->mTCPSocket->mSocket);
		if (session == nullptr)
		{
			GLogger->PrintConsole(Color::RED, L"%d Client Session Not Found(Send)", ioContext->mTCPSocket->mSocket);
			break;
		}

		session->SendCompletion(transferred);

		break;
	}
	}

	DeleteIOContext(ioContext);

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void IOCPThread::SendProcess()
{
	mClientSessionManager->FlushSendClientSessionAll();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void IOCPThread::Init(const HANDLE iocpHandle, ClientSessionManager* clientSessionManager, std::function<void(const Packet)> pushPacketFunction)
{
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
				SendProcess();
			}
		}
	);
}
