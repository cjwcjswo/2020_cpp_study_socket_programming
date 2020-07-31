#pragma once

#include "ErrorCode.h"
#include "Protocol.h"
#include "../../NetworkLib/Protocol.h"

class NetworkCore;
class UserManager;

class PacketHandler
{
private:
	using Packet = Core::ReceivePacket;
	using PacketFunc = CS::ErrorCode (PacketHandler::*)(const Packet);


private:
	NetworkCore* mNetworkCore;
	UserManager* mUserManager;

	PacketFunc mPacketFuncArray[CS::PACKET_ID_END - CS::PACKET_ID_START + 1];

	void EnrollPacketFunc(CS::PacketId packetId, PacketFunc);

public:
	explicit PacketHandler(NetworkCore* networkCore, UserManager* userManager);


private:
	CS::ErrorCode Connect(const Packet packet);
	CS::ErrorCode Disconnect(const Packet packet);
	CS::ErrorCode Chat(const Packet packet);

	
public:
	CS::ErrorCode Process(const Packet packet);
};

