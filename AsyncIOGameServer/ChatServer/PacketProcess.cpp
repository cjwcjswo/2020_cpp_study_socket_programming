#include "../NetworkLib/Network.h"
#include "../NetworkLib/Logger.h"
#include "PacketHandler.h"
#include "UserManager.h"
#include "User.h"


using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode PacketHandler::Connect(const Packet& packet)
{
	User newUser{ packet.mSessionIndex, packet.mSessionUniqueId };

	ErrorCode errorCode = mUserManager->Connect(newUser);
	if (errorCode != ErrorCode::SUCCESS)
	{
		return errorCode;
	}

	GLogger->PrintConsole(Color::LGREEN, L"<Connect> User: %lu\n", packet.mSessionUniqueId);

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode PacketHandler::Disconnect(const Packet& packet)
{
	User* user = mUserManager->FindUser(packet.mSessionUniqueId);
	if (user != nullptr)
	{
		mUserManager->Disconnect(user->mIndex);
	}

	GLogger->PrintConsole(Color::LGREEN, L"<Disconnect> User: %lu\n", packet.mSessionUniqueId);

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode PacketHandler::Login(const Packet& packet)
{
	User* user = mUserManager->FindUser(packet.mSessionUniqueId);
	if (user == nullptr)
	{
		LoginResponse response;
		response.mErrorCode = ErrorCode::USER_IS_INVALID;
		mNetwork->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::LOGIN_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));
		return response.mErrorCode;
	}

	LoginRequest* request = reinterpret_cast<LoginRequest*>(packet.mBodyData);
	LoginResponse response;

#ifndef _DEBUG
	Redis::CommandRequest commandRequest;
	commandRequest.mCommandType = Redis::CommandType::GET;
	Redis::Get get;
	strcpy_s(get.mKey, CS::RedisLoginKey(request->mUserId).c_str());
	commandRequest.mCommandBody = reinterpret_cast<char*>(&get);
	commandRequest.mCommandBodySize = sizeof(get);
	mRedisManager->ExecuteCommandAsync(commandRequest);
	Redis::CommandResponse commandResponse = mRedisManager->GetCommandResult();

	if (commandResponse.mErrorCode != ErrorCode::SUCCESS)
	{
		response.mErrorCode = ErrorCode::USER_LOGIN_AUTH_FAIL;
		mNetwork->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::LOGIN_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));
		return response.mErrorCode;
	}
	if (std::strncmp(request->mAuthKey, commandResponse.mResult.c_str(), AUTH_KEY_SIZE) != 0)
	{
		response.mErrorCode = ErrorCode::USER_LOGIN_AUTH_FAIL;
		mNetwork->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::LOGIN_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));
		return response.mErrorCode;
	}
#endif

	response.mErrorCode = mUserManager->Login(packet.mSessionUniqueId, request->mUserId);
	mNetwork->Send(packet.mSessionIndex, static_cast<uint16>(PacketId::LOGIN_RESPONSE), reinterpret_cast<char*>(&response), sizeof(response));

	GLogger->PrintConsole(Color::LGREEN, L"<Login> User: %lu ErrorCode: %d\n", packet.mSessionUniqueId, static_cast<int>(response.mErrorCode));

	return ErrorCode::SUCCESS;
}