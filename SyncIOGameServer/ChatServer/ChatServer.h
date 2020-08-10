#pragma once

#include "ErrorCode.h"


namespace NetworkLib 
{
	class Network;
}
namespace Redis
{
	class RedisManager;
}
class UserManager;
class RoomManager;
class PacketHandler;
class Config;


class ChatServer
{
private:
	NetworkLib::Network* mNetwork = nullptr;
	Redis::RedisManager* mRedisManager = nullptr;
	UserManager* mUserManager = nullptr;
	RoomManager* mRoomManager = nullptr;
	
	PacketHandler* mPacketHandler = nullptr;
	Config* mConfig = nullptr;

	bool mIsRunning = false;


public:
	ChatServer() = default;
	~ChatServer();


public:
	CS::ErrorCode Init();
	CS::ErrorCode Run();
	void Stop();
};