#include <wchar.h>

#include "PacketHandler.h"
#include "UserManager.h"
#include "User.h"
#include "../../NetworkLib/NetworkCore.h"


using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
PacketHandler::PacketHandler(NetworkCore* networkCore, UserManager* userManager)
{
	mNetworkCore = networkCore;
	mUserManager = userManager;

	for (int i = 0; i < PACKET_ID_END - PACKET_ID_START + 1; ++i)
	{
		mPacketFuncArray[i] = nullptr;
	}

	EnrollPacketFunc(PacketId::CHAT_REQUEST, &PacketHandler::Chat);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PacketHandler::EnrollPacketFunc(PacketId packetId, PacketFunc packetFunc)
{
	int index = static_cast<int>(packetId) - (PACKET_ID_START + 1);
	mPacketFuncArray[index] = packetFunc;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode PacketHandler::Process(const Packet packet)
{
	if (packet.mPacketId > Core::PACKET_ID_START && packet.mPacketId < Core::PACKET_ID_END)
	{
		if (packet.mPacketId == static_cast<uint16>(Core::PacketId::CONNECT))
		{
			return Connect(packet);
		}

		if (packet.mPacketId == static_cast<uint16>(Core::PacketId::DISCONNECT))
		{
			return Disconnect(packet);
		}

		return ErrorCode::CHAT_SERVER_API_NOT_EXIST;
	}

	if (packet.mPacketId < PACKET_ID_START || packet.mPacketId > PACKET_ID_END)
	{
		return ErrorCode::CHAT_SERVER_INVALID_API;
	}

	int packetIndex = static_cast<int>(packet.mPacketId) - (PACKET_ID_START + 1);
	PacketFunc packetFunc = mPacketFuncArray[packetIndex];
	if (packetFunc == nullptr)
	{
		return ErrorCode::CHAT_SERVER_API_NOT_EXIST;
	}

	return (this->*packetFunc)(packet);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode PacketHandler::Connect(const Packet packet)
{
	User newUser{ packet.mSessionIndex, packet.mSessionUniqueId, packet.mSessionUniqueId };

	ErrorCode errorCode = mUserManager->Connect(newUser);
	if (ErrorCode::SUCCESS != errorCode)
	{
		return errorCode;
	}

	GLogger->PrintConsole(Color::LGREEN, L"<Connect> User: %lu\n", packet.mSessionUniqueId);

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode PacketHandler::Disconnect(const Packet packet)
{
	User* user = mUserManager->FindUser(packet.mSessionUniqueId);
	if (nullptr != user)
	{
		mUserManager->Disconnect(user->mIndex);
	}

	GLogger->PrintConsole(Color::LGREEN, L"<Disconnect> User: %lu\n", packet.mSessionUniqueId);

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode PacketHandler::Chat(const Packet packet)
{
	ChatRequest* request = reinterpret_cast<ChatRequest*>(packet.mBodyData);

	ChatBroadcast broadcast;
	broadcast.mUid = packet.mSessionUniqueId;
	broadcast.mMessageLen = request->mMessageLen;
	wmemcpy_s(broadcast.mMessage, request->mMessageLen, request->mMessage, request->mMessageLen);
	mNetworkCore->Broadcast(static_cast<uint16>(PacketId::CHAT_BROADCAST), reinterpret_cast<char*>(&broadcast), sizeof(broadcast) - (MAX_CHAT_SIZE - broadcast.mMessageLen));

	GLogger->PrintConsole(Color::LGREEN, L"<Chat> [%lu]: %ls\n", 5, broadcast.mMessage);

	ChatResponse response;
	response.mErrorCode = ErrorCode::SUCCESS;
	mNetworkCore->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::CHAT_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));

	return ErrorCode::SUCCESS;
}