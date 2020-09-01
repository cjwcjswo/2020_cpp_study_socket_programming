#pragma once

#include <thread>
#include "ErrorCode.h"


namespace NetworkLib
{
	class Network;
}
namespace Redis
{
	class Manager;
}
class UserManager;
class RoomManager;
class PacketHandler;
class Config;


class ChatServer
{
private:
	NetworkLib::Network* mNetwork = nullptr;
	Redis::Manager* mRedisManager = nullptr;
	UserManager* mUserManager = nullptr;
	RoomManager* mRoomManager = nullptr;

	PacketHandler* mPacketHandler = nullptr;
	Config* mConfig = nullptr;

	std::unique_ptr<std::thread> mNetworkThread = nullptr;
	bool mIsRunning = false;


public:
	ChatServer() = default;
	~ChatServer();


public:
	CS::ErrorCode Init();
	CS::ErrorCode Run();
	void Stop();
};