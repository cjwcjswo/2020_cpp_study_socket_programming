#include <cstring>
#include <wchar.h>

#include "../../NetworkLib/NetworkCore.h"
#include "../../NetworkLib/Logger.h"
#include "PacketHandler.h"
#include "RedisManager.h"
#include "UserManager.h"
#include "User.h"


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
	User newUser{ packet.mSessionIndex, packet.mSessionUniqueId };

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
ErrorCode PacketHandler::Login(const Packet packet)
{
	LoginResponse response;
	response.mErrorCode = ErrorCode::SUCCESS;

	User* user = mUserManager->FindUser(packet.mSessionUniqueId);
	if (nullptr == user)
	{
		response.mErrorCode = ErrorCode::INVALID_USER;
		mNetworkCore->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::LOGIN_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));
		return response.mErrorCode;
	}

	LoginRequest* request = reinterpret_cast<LoginRequest*>(packet.mBodyData);

	//TODO 최흥배: Redis를 다룰 때는 Redis 전용 스레드를 만들어서 그쪽에서 해야 합니다. redis가 io 대기를 발생시켜서 서버 성능에 나쁜 영향을 줍니다.
	// 참고: https://docs.google.com/presentation/d/16DgIURxfR9jgHjLX7fCwruHT-vwm90BG1OkQVdE0j9A/edit?usp=sharing
	RedisResult redisResult =  GRedisManager->Get(CS::RedisLoginKey(request->mUid));
	if (ErrorCode::SUCCESS != redisResult.mErrorCode)
	{
		response.mErrorCode = redisResult.mErrorCode;
		mNetworkCore->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::LOGIN_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));
		return response.mErrorCode;
	}
	if (0 != std::strncmp(request->mAuthKey, redisResult.mResult, AUTH_KEY_SIZE))
	{
		response.mErrorCode = ErrorCode::LOGIN_AUTH_FAIL;
		mNetworkCore->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::LOGIN_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));
		return response.mErrorCode;
	}

	mUserManager->Login(packet.mSessionUniqueId, request->mUid);

	mNetworkCore->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::LOGIN_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));

	GLogger->PrintConsole(Color::LGREEN, L"<Login> User: %lu\n", packet.mSessionUniqueId);

	return response.mErrorCode;
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