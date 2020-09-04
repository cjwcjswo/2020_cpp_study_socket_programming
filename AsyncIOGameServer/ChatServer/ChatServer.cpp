#include "ChatServer.h"

#include "../NetworkLib/Network.h"
#include "../NetworkLib/Logger.h"
#include "Config.h"
#include "UserManager.h"
#include "User.h"
#include "RoomManager.h"
#include "PacketHandler.h"


using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChatServer::~ChatServer()
{
	if (mNetwork != nullptr)
	{
		delete mNetwork;
	}
	if (mUserManager != nullptr)
	{
		delete mUserManager;
	}
	if (mRoomManager != nullptr)
	{
		delete mRoomManager;
	}
	if (mPacketHandler != nullptr)
	{
		delete mPacketHandler;
	}
	if (mConfig != nullptr)
	{
		delete mConfig;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ChatServer::Init()
{
	mNetwork = new NetworkLib::Network();
	if (mNetwork->Init() != NetworkLib::ErrorCode::SUCCESS)
	{
		return ErrorCode::CHAT_SERVER_INIT_FAIL;
	}

	mConfig = new Config();
	mConfig->Load();

	mUserManager = new UserManager;
	mUserManager->Init(mConfig->mMaxUserNum);

	mRoomManager = new RoomManager;
	mRoomManager->Init(mConfig->mMaxRoomUserNum, mConfig->mMaxRoomUserNum);

	// TODO: DBManager 완성 후 작성
	/*mRedisManager = new Redis::Manager(mConfig->mRedisCheckSendTick, mConfig->mRedisCheckReceiveTick, mConfig->mRedisCheckReceiveTimeOut);
	ErrorCode errorCode = mRedisManager->Connect(mConfig->mRedisAddress.c_str(), mConfig->mRedisPortNum);
	if (errorCode != ErrorCode::SUCCESS)
	{
		return ErrorCode::CHAT_SERVER_INIT_FAIL;
	}*/

	mPacketHandler = new PacketHandler(mNetwork, mUserManager, mRoomManager, mRedisManager);

	GLogger->PrintConsole(Color::GREEN, L"ChatServer Init Success\n");

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ChatServer::Run()
{
	mIsRunning = true;

	// TODO 최흥배
	// Run 함수의 마지막에서 WaitForSingleObject(mExitEvent, INFINITE); 호출 때문에 스레드를 만들어서 Run을 호출하고 있는데
	// 올바르지 않습니다. WaitForSingleObject(mExitEvent, INFINITE)로 대기를 하는 의미를 모르겠습니다
	// 적용 완료
	mNetwork->Run();

	GLogger->PrintConsole(Color::GREEN, L"ChatServer Start\n");

	while (mIsRunning)
	{
		NetworkLib::Packet receivePacket = mNetwork->GetReceivePacket();
		if (receivePacket.mPacketId == 0)
		{
			continue;
		}

		ErrorCode errorCode = mPacketHandler->Process(receivePacket);
		if (errorCode != ErrorCode::SUCCESS)
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
	mNetwork->Stop();
}