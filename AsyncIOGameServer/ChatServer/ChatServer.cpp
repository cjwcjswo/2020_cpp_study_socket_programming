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

	// TODO: DBManager �ϼ� �� �ۼ�
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

	// TODO �����
	// Run �Լ��� ���������� WaitForSingleObject(mExitEvent, INFINITE); ȣ�� ������ �����带 ���� Run�� ȣ���ϰ� �ִµ�
	// �ùٸ��� �ʽ��ϴ�. WaitForSingleObject(mExitEvent, INFINITE)�� ��⸦ �ϴ� �ǹ̸� �𸣰ڽ��ϴ�
	// ���� �Ϸ�
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