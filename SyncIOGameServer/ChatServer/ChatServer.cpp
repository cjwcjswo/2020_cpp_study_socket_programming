#include "ChatServer.h"

using namespace CS;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ChatServer::Init()
{
	Core::ErrorCode errorCode = mNetworkCore.Init();
	if (Core::ErrorCode::SUCCESS != errorCode)
	{
		return ErrorCode::CHAT_SERVER_INIT_FAIL;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ChatServer::Run()
{
	mIsRunning = true;
	mNetworkCore.Run(); // Async

	while (mIsRunning)
	{
		Core::ReceivePacket receivePacket = mNetworkCore.GetReceivePacket();
		if (0 == receivePacket.mPacketId)
		{
			continue;
		}
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ChatServer::Stop()
{
	mIsRunning = false;
}