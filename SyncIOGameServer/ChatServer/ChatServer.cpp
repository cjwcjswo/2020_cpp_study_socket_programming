#include "../../NetworkLib/Network.h"
#include "../../NetworkLib/Logger.h"
#include "ChatServer.h"
#include "UserManager.h"
#include "User.h"
#include "PacketHandler.h"
#include "RedisManager.h"


using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChatServer::~ChatServer()
{
	delete mNetwork;
	delete mUserManager;
	delete mPacketHandler;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ChatServer::Init()
{
	mNetwork = new NetworkLib::Network();
	if (NetworkLib::ErrorCode::SUCCESS != mNetwork->Init())
	{
		return ErrorCode::CHAT_SERVER_INIT_FAIL;
	}

	mUserManager = new UserManager;
	mUserManager->Init(100);

	mPacketHandler = new PacketHandler(mNetwork, mUserManager);

	Redis::GRedisManager = new Redis::RedisManager();
	ErrorCode errorCode = Redis::GRedisManager->Connect();
	if (ErrorCode::SUCCESS != errorCode)
	{
		return errorCode;
	}

	GLogger->PrintConsole(Color::GREEN, L"ChatServer Init Success\n");
	
	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ChatServer::Run()
{
	mIsRunning = true;
	mNetwork->Run(); // Async

	GLogger->PrintConsole(Color::GREEN, L"ChatServer Start\n");

	while (mIsRunning)
	{
		NetworkLib::ReceivePacket receivePacket = mNetwork->GetReceivePacket();
		if (0 == receivePacket.mPacketId)
		{
			continue;
		}
		
		// TODO 최진우: 동시성 문제로 인해 버퍼 복사, 더 좋은 방법이 없을까?
		char copyBodyBuffer[1024] = { 0, };
		if (receivePacket.mBodyDataSize > 0)
		{
			memcpy_s(copyBodyBuffer, receivePacket.mBodyDataSize, receivePacket.mBodyData, receivePacket.mBodyDataSize);
			receivePacket.mBodyData = copyBodyBuffer;
		}

		ErrorCode errorCode = mPacketHandler->Process(receivePacket);
		if (ErrorCode::SUCCESS != errorCode)
		{
			GLogger->PrintConsole(Color::RED, L"Packet Process Error(%d) / [PacketId: %u, UniqueId: %u]\n",
				static_cast<int>(errorCode), receivePacket.mPacketId, receivePacket.mSessionUniqueId);
		}
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ChatServer::Stop()
{
	GLogger->PrintConsole(Color::GREEN, L"ChatServer Stop\n");

	mIsRunning = false;
}