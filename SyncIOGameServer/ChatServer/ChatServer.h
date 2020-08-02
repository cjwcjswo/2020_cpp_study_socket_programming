#pragma once

#include "ErrorCode.h"


class NetworkCore;
class UserManager;
class PacketHandler;


class ChatServer
{
private:
	NetworkCore* mNetworkCore = nullptr;
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