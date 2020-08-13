#include <cstring>
#include <string>
#include <wchar.h>

#include "../../NetworkLib/Network.h"
#include "../../NetworkLib/Logger.h"
#include "../../NetworkLib/RedisManager.h"
#include "PacketHandler.h"
#include "UserManager.h"
#include "User.h"


using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PacketHandler::PacketHandler(NetworkLib::Network* network, UserManager* userManager, RoomManager* roomManager, NetworkLib::Redis::Manager* redisManager)
{
	mNetwork = network;
	mRedisManager = redisManager;
	mUserManager = userManager;
	mRoomManager = roomManager;

	for (int i = 0; i < PACKET_ID_END - PACKET_ID_START + 1; ++i)
	{
		mPacketFuncArray[i] = nullptr;
	}

	EnrollPacketFunc(PacketId::LOGIN_REQUEST, &PacketHandler::Login);
	EnrollPacketFunc(PacketId::ROOM_ENTER_REQUEST, &PacketHandler::RoomEnter);
	EnrollPacketFunc(PacketId::ROOM_LEAVE_REQUEST, &PacketHandler::RoomLeave);
	EnrollPacketFunc(PacketId::ROOM_CHAT_REQUEST, &PacketHandler::RoomChat);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PacketHandler::EnrollPacketFunc(PacketId packetId, PacketFunc packetFunc)
{
	int index = static_cast<int>(packetId) - (PACKET_ID_START + 1);
	mPacketFuncArray[index] = packetFunc;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode PacketHandler::Process(const Packet& packet)
{
	if (packet.mPacketId > NetworkLib::PACKET_ID_START && packet.mPacketId < NetworkLib::PACKET_ID_END)
	{
		if (static_cast<uint16>(NetworkLib::PacketId::CONNECT) == packet.mPacketId)
		{
			return Connect(packet);
		}

		if (static_cast<uint16>(NetworkLib::PacketId::DISCONNECT) == packet.mPacketId)
		{
			return Disconnect(packet);
		}

		return ErrorCode::CHAT_SERVER_API_NOT_EXIST;
	}

	if (packet.mPacketId < PACKET_ID_START || packet.mPacketId > PACKET_ID_END)
	{
		return ErrorCode::CHAT_SERVER_INVALID_API;
	}

	if (static_cast<uint16>(PacketId::LOGIN_REQUEST) != packet.mPacketId)
	{
		User* user = mUserManager->FindUser(packet.mSessionUniqueId);
		if (nullptr == user)
		{
			return ErrorCode::USER_NOT_CONNECTED_STATE;
		}
		if (UserState::DISCONNECT == user->mState || UserState::CONNECT == user->mState)
		{
			return ErrorCode::USER_NOT_LOGIN_STATE;
		}
	}

	int packetIndex = static_cast<int>(packet.mPacketId) - (PACKET_ID_START + 1);
	PacketFunc packetFunc = mPacketFuncArray[packetIndex];
	if (packetFunc == nullptr)
	{
		return ErrorCode::CHAT_SERVER_API_NOT_EXIST;
	}

	return (this->*packetFunc)(packet);
}