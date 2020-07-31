#include "ChatServer.h"
#include "UserManager.h"
#include "User.h"
#include "PacketHandler.h"

using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ChatServer::~ChatServer()
{
	delete mUserManager;
	delete mPacketHandler;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ChatServer::Init()
{
	Core::ErrorCode errorCode = mNetworkCore.Init(150);
	if (Core::ErrorCode::SUCCESS != errorCode)
	{
		return ErrorCode::CHAT_SERVER_INIT_FAIL;
	}

	mUserManager = new UserManager;
	mUserManager->Init(100);

	mPacketHandler = new PacketHandler(&mNetworkCore, mUserManager);

	GLogger->PrintConsole(Color::GREEN, L"ChatServer Init Success\n");
	
	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ChatServer::Run()
{
	mIsRunning = true;
	mNetworkCore.Run(); // Async

	GLogger->PrintConsole(Color::GREEN, L"ChatServer Start\n");

	while (mIsRunning)
	{
		Core::ReceivePacket receivePacket = mNetworkCore.GetReceivePacket();
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