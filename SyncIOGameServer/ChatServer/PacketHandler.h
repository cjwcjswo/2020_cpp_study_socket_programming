#pragma once

#include "../../NetworkLib/Protocol.h"
#include "ErrorCode.h"
#include "Protocol.h"


class NetworkLib::Network;
class UserManager;
class RoomManager;


class PacketHandler
{
private:
	using Packet = NetworkLib::ReceivePacket;
	using PacketFunc = CS::ErrorCode (PacketHandler::*)(const Packet&);


private:
	NetworkLib::Network* mNetwork = nullptr;
	UserManager* mUserManager = nullptr;
	RoomManager* mRoomManager = nullptr;

	PacketFunc mPacketFuncArray[CS::PACKET_ID_END - CS::PACKET_ID_START + 1];

	void EnrollPacketFunc(CS::PacketId packetId, PacketFunc);


public:
	explicit PacketHandler(NetworkLib::Network* network, UserManager* userManager, RoomManager* roomManager);


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

