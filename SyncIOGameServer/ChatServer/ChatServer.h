#pragma once

#include "../../NetworkLib/NetworkCore.h"
#include "ErrorCode.h"

class UserManager;
class PacketHandler;

class ChatServer
{
private:
	NetworkCore mNetworkCore;

	UserManager* mUserManager = nullptr;
	PacketHandler* mPacketHandler = nullptr;
	

	bool mIsRunning = false;

public:
	ChatServer() = default;
	~ChatServer();


public:
	CS::ErrorCode Init();
	CS::ErrorCode Run();
	void Stop();

};