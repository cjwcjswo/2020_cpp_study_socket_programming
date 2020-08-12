#pragma once

#include <string>

#include "../../NetworkLib/PrimitiveTypes.h"
#include "../../NetworkLib/Define.h"
#include "ErrorCode.h"


namespace CS 
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const int PACKET_ID_START = 100;
	const int PACKET_ID_END = 10000;


	enum class PacketId : uint16
	{
		PACKET_START = PACKET_ID_START,
		//////////////////////////////////////////////////////////////
		LOGIN_REQUEST = 101,
		LOGIN_RESPONSE = 102,

		//////////////////////////////////////////////////////////////
		ROOM_PACKET_START = 110,

		ROOM_ENTER_REQUEST = 111,
		ROOM_ENTER_RESPONSE = 112,
		ROOM_NEW_USER_BROADCAST = 113,
		ROOM_USER_LIST_NOTIFY = 114,

		ROOM_LEAVE_REQUEST = 115,
		ROOM_LEAVE_RESPONSE = 116,
		ROOM_LEAVE_USER_BROADCAST= 117,

		ROOM_PACKET_END = 200,
		//////////////////////////////////////////////////////////////
		CHAT_REQUEST = 201,
		CHAT_RESPONSE = 202,
		CHAT_BROADCAST = 203,


		//////////////////////////////////////////////////////////////
		PACKET_END = PACKET_ID_END,
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct PacketBase
	{
		ErrorCode mErrorCode = ErrorCode::SUCCESS;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const int MAX_USER_ID_SIZE = 16;
	const int AUTH_KEY_SIZE = 64;

#pragma pack(push, 1)
	struct LoginRequest
	{
		char mUserId[MAX_USER_ID_SIZE] = { 0, };
		char mAuthKey[AUTH_KEY_SIZE] = { 0, };
	};

	struct LoginResponse: PacketBase
	{
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const int16 MAX_CHAT_SIZE = 128;

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

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct RoomEnterRequest
	{
		int32 mRoomIndex = INVALID_INDEX;
	};
	
	struct RoomEnterResponse : PacketBase
	{
		uint64 mUserUniqueId = INVALID_UNIQUE_ID;
	};

	
	struct RoomUserListNotify
	{
		struct RoomUserInfo
		{
			uint64 mUserUniqueId = INVALID_UNIQUE_ID;
			char mUserIdList[MAX_USER_ID_SIZE] = { 0, };
		};
		uint16 mUserCount = 0;
		RoomUserInfo mRoomUserList[1];


		static int Size(uint16 userCount)
		{
			return sizeof(uint16) + (userCount * sizeof(uint64)) + (userCount * sizeof(char) * MAX_USER_ID_SIZE);
		}
	};

	struct RoomNewUserBroadcast
	{
		uint64 mUserUniueId = INVALID_UNIQUE_ID;
		char mUserId[MAX_USER_ID_SIZE] = { 0, };
	};

	struct RoomLeaveRequest
	{
	};

	struct RoomLeaveResponse : PacketBase
	{
	};

	struct RoomLeaveUserBroadcast
	{
		uint64 mUserUniqueId = INVALID_UNIQUE_ID;
	};
#pragma pack(pop)
}


namespace CS
{
	inline std::string RedisLoginKey(const char* userId)
	{
		return std::string{ std::string{"Login/"} + userId };
	}
}