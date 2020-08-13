#include "../../NetworkLib/Network.h"
#include "../../NetworkLib/Logger.h"
#include "RedisManager.h"
#include "Config.h"
#include "ChatServer.h"
#include "UserManager.h"
#include "User.h"
#include "RoomManager.h"
#include "PacketHandler.h"


using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChatServer::~ChatServer()
{
	if (nullptr != mNetwork)
	{
		delete mNetwork;
	}
	if (nullptr != mUserManager)
	{
		delete mUserManager;
	}
	if (nullptr != mRoomManager)
	{
		delete mRoomManager;
	}
	if (nullptr != mPacketHandler)
	{
		delete mPacketHandler;
	}
	if (nullptr != mConfig)
	{
		delete mConfig;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ChatServer::Init()
{
	mNetwork = new NetworkLib::Network();
	if (NetworkLib::ErrorCode::SUCCESS != mNetwork->Init())
	{
		return ErrorCode::CHAT_SERVER_INIT_FAIL;
	}

	mConfig = new Config();
	mConfig->Load();

	mUserManager = new UserManager;
	mUserManager->Init(mConfig->mMaxUserNum);

	mRoomManager = new RoomManager;
	mRoomManager->Init(mConfig->mMaxRoomUserNum);

	mRedisManager = new Redis::Manager(mConfig->mRedisCheckSendTick, mConfig->mRedisCheckReceiveTick, mConfig->mRedisCheckReceiveTimeOut);
	ErrorCode errorCode = mRedisManager->Connect(mConfig->mRedisAddress.c_str(), mConfig->mRedisPortNum);
	if (ErrorCode::SUCCESS != errorCode)
	{
		return ErrorCode::CHAT_SERVER_INIT_FAIL;
	}

	mPacketHandler = new PacketHandler(mNetwork, mUserManager, mRoomManager, mRedisManager);

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
		NetworkLib::Packet receivePacket = mNetwork->GetReceivePacket();
		if (0 == receivePacket.mPacketId)
		{
			continue;
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