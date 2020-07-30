#include "PacketHandler.h"
#include "UserManager.h"
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
	EnrollPacketFunc(PacketId::ChatRequest, &PacketHandler::Chat);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void PacketHandler::EnrollPacketFunc(PacketId packetId, PacketFunc packetFunc)
{
	int index = static_cast<int>(packetId) - (PACKET_ID_START + 1);
	mPacketFuncArray[index] = packetFunc;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode PacketHandler::Process(Packet packet)
{
	if (packet.mPacketId <= PACKET_ID_START || packet.mPacketId >= PACKET_ID_END)
	{
		return ErrorCode::CHAT_SERVER_API_NOT_EXIST;
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
ErrorCode PacketHandler::Chat(Packet packet)
{
	ChatRequest* request = reinterpret_cast<ChatRequest*>(packet.mBodyData);

	ChatBroadcast broadcast;
	broadcast.mUid = packet.mSessionUniqueId;
	memcpy_s(&broadcast.mMessage, request->mMessageLen, &request->mMessage, request->mMessageLen);
	mNetworkCore->Broadcast(static_cast<uint16>(PacketId::ChatBroadcast), reinterpret_cast<char*>(&broadcast), sizeof(broadcast) - (MAX_CHAT_SIZE - broadcast.mMessageLen));

	ChatResponse response;
	response.mErrorCode = ErrorCode::SUCCESS;
	mNetworkCore->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::ChatResponse), reinterpret_cast<char*>(&response), sizeof(response));

	return ErrorCode::SUCCESS;
}