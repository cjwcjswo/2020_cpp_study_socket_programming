#include "Network.h"

#include "OverlappedIOContext.h"
#include "IOCPThread.h"
#include "TCPSocket.h"
#include "Logger.h"
#include "ClientSessionManager.h"
#include "ClientSession.h"
#include "Config.h"


using namespace NetworkLib;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Network::PushReceivePacket(const Packet receivePacket)
{
	std::lock_guard<std::mutex> lock(mReceivePacketMutex);
	mReceivePacketQueue.push(receivePacket);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Network::Init()
{
	mConfig = new Config();
	ErrorCode errorCode = mConfig->Load();
	if (errorCode != ErrorCode::SUCCESS)
	{
		return errorCode;
	}

	WSADATA wsaData;

	if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR)
	{
		return ErrorCode::WSA_START_UP_FAIL;
	}

	mListenSocket = new TCPSocket(mConfig->mSocketAddressBufferSize);

	errorCode = mListenSocket->Create();
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
	errorCode = mClientSessionManager->Init(mConfig->mMaxSessionNum, mConfig->mMaxSessionBufferSize, mConfig->mSocketAddressBufferSize, mConfig->mSpinLockCount);
	if (errorCode != ErrorCode::SUCCESS)
	{
		GLogger->PrintConsole(Color::RED, L"ClientSessionManager Init Error: %d\n", static_cast<uint16>(errorCode));
		return errorCode;
	}

	mIOCPThreads = new IOCPThread[mConfig->mMaxThreadNum];

	if (!GIOContextPool->Create(mConfig->mIOContextPoolSize))
	{
		GLogger->PrintConsole(Color::RED, L"IOCPContextPool Create Error: %d\n", static_cast<uint16>(ErrorCode::IO_CONTEXT_POOL_CREATE_FAIL));
		return ErrorCode::IO_CONTEXT_POOL_CREATE_FAIL;
	}
	for (uint32 i = 0; i < mConfig->mIOContextPoolSize; ++i)
	{
		GIOContextPool->Push(new OverlappedIOContext);
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Network::Run()
{
	GLogger->PrintConsole(Color::LBLUE, "Run Server~~~~~\n");

	mIOCPHandle = CreateIoCompletionPort(reinterpret_cast<HANDLE>(mListenSocket->mSocket), nullptr, static_cast<ULONG_PTR>(IOKey::ACCEPT), 0);

	for (uint32 i = 0; i < mConfig->mMaxThreadNum; ++i)
	{
		mIOCPThreads[i].Init(&mIsRunning, mListenSocket, mIOCPHandle, mClientSessionManager, [this](const Packet packet) {PushReceivePacket(packet); });
		mIOCPThreads[i].Run();
	}

	mSendThread = std::make_unique<std::thread>(
		[this]() 
		{
			while (true)
			{
				mClientSessionManager->FlushSendClientSessionAll();
				Sleep(mConfig->mSendPacketCheckTick);
			}
		}
	);

	// Reserve Accept Async
	ErrorCode errorCode;
	for (uint32 i = 0; i < mConfig->mMaxThreadNum; ++i)
	{
		ClientSession* session = mClientSessionManager->FindClientSession(static_cast<int32>(i));
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

	mIsRunning = true;

	return ErrorCode::SUCCESS;
}

void Network::Stop()
{
	GLogger->PrintConsole(Color::LBLUE, "Stop Server~~~~~~\n");

	mIsRunning = false;

	CloseHandle(mExitEvent);

	for (uint32 i = 0; i < mConfig->mMaxThreadNum; ++i)
	{
		mIOCPThreads[i].Join();
	}
	mSendThread->join();

	WSACleanup();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Network::SendRawData(const int32 sessionIndex, char* data, const uint16 dataSize)
{
	ClientSession* session = mClientSessionManager->FindClientSession(sessionIndex);
	if (session == nullptr)
	{
		return;
	}

	session->SendAsync(data, dataSize);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Network::Send(const int32 sessionIndex, const uint16 packetId, char* bodyData, const uint16 bodySize)
{
	ClientSession* session = mClientSessionManager->FindClientSession(sessionIndex);
	if (session == nullptr)
	{
		return;
	}

	uint16 totalSize = PACKET_HEADER_SIZE + bodySize;
	PacketData packetData;
	packetData.mHeader = PacketHeader{ totalSize, packetId };
	memcpy_s(packetData.mBodyData, bodySize, bodyData, bodySize);
	session->SendAsync(reinterpret_cast<char*>(&packetData), totalSize);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Packet Network::GetReceivePacket()
{
	std::lock_guard<std::mutex> lock(mReceivePacketMutex);

	if (mReceivePacketQueue.empty())
	{
		return Packet{};
	}
	Packet receivePacket = mReceivePacketQueue.front();
	mReceivePacketQueue.pop();

	return receivePacket;
}
