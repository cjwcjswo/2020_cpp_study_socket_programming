#pragma once

#include "../../NetworkLib/Protocol.h"
#include "ErrorCode.h"
#include "Protocol.h"


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
class RedisManager;


class PacketHandler
{
private:
	using Packet = NetworkLib::Packet;
	using PacketFunc = CS::ErrorCode (PacketHandler::*)(const Packet&);


private:
	NetworkLib::Network* mNetwork = nullptr;
	Redis::RedisManager* mRedisManager = nullptr;
	UserManager* mUserManager = nullptr;
	RoomManager* mRoomManager = nullptr;

	PacketFunc mPacketFuncArray[CS::PACKET_ID_END - CS::PACKET_ID_START + 1];

	void EnrollPacketFunc(CS::PacketId packetId, PacketFunc);


public:
	explicit PacketHandler(NetworkLib::Network* network, UserManager* userManager, RoomManager* roomManager, Redis::RedisManager* redisManager);


private:
	// Packet Process
	CS::ErrorCode Connect(const Packet& packet);
	CS::ErrorCode Disconnect(const Packet& packet);
	CS::ErrorCode Login(const Packet& packet);
	CS::ErrorCode Chat(const Packet& packet);


	// Packet Process Room
	CS::ErrorCode RoomEnter(const Packet& packet);
	CS::ErrorCode RoomLeave(const Packet& packet);
	CS::ErrorCode RoomChat(const Packet& packet);
	
public:
	CS::ErrorCode Process(const Packet& packet);
};

