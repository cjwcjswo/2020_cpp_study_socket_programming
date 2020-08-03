#pragma once

#include <string>

#include "../../NetworkLib/PrimitiveTypes.h"
#include "ErrorCode.h"


namespace CS 
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	constexpr static int PACKET_ID_START = 100;
	constexpr static int PACKET_ID_END = 10000;

	enum class PacketId : uint16
	{
		PACKET_START = PACKET_ID_START,
		//////////////////////////////////////////////////////////////
		LOGIN_REQUEST,
		LOGIN_RESPONSE,

		CHAT_REQUEST,
		CHAT_RESPONSE,
		CHAT_BROADCAST,


		//////////////////////////////////////////////////////////////
		PACKET_END = PACKET_ID_END,
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct PacketBase
	{
		ErrorCode mErrorCode = ErrorCode::SUCCESS;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	constexpr static int AUTH_KEY_SIZE = 64;

	struct LoginRequest
	{
		uint64 mUid = 0;
		char mAuthKey[AUTH_KEY_SIZE] = { 0, };
	};

	struct LoginResponse: PacketBase
	{
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	constexpr static int16 MAX_CHAT_SIZE = 128;

	struct ChatRequest
	{
		uint16 mMessageLen = 0;
		wchar mMessage[MAX_CHAT_SIZE] = { 0, };
	};

	struct ChatResponse : PacketBase
	{
	};

	struct ChatBroadcast
	{
		uint64 mUid = 0;
		uint16 mMessageLen = 0;
		wchar mMessage[MAX_CHAT_SIZE] = { 0, };
	};
}


namespace CS
{
	inline const char* RedisLoginKey(uint64 uid)
	{
		return std::string("Login/" + std::to_string(uid)).c_str();
	}
}