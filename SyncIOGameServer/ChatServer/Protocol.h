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
		LOGIN_REQUEST,
		LOGIN_RESPONSE,

		//////////////////////////////////////////////////////////////
		ROOM_PACKET_START,

		ROOM_ENTER_REQUEST,
		ROOM_ENTER_RESPONSE,
		ROOM_NEW_USER_BROADCAST,
		ROOM_USER_LIST_NOTIFY,

		ROOM_LEAVE_REQUEST,
		ROOM_LEAVE_RESPONSE,
		ROOM_LEAVE_USER_BROADCAST,

		ROOM_PACKET_END,
		//////////////////////////////////////////////////////////////
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
		uint16 mUserCount = 0;
		uint64* mUserUniqueIdList = nullptr;
		char** mUserIdList = nullptr;


		int Size()
		{
			return sizeof(uint16) + (mUserCount * sizeof(uint64)) + (mUserCount * sizeof(char) * MAX_USER_ID_SIZE);
		}
	};

	//TODO 최흥배. 구조체 그대로 바이트배열로 형변환 하고 있는데 이럴 경우 1바이트 정렬 시키지 않으면 구조체 패딩이 발생합니다. 구조체 크기를 그대로 sizeof로 계산해서 패킷 데이터 크기를 계산하는 경우는 모두 조심하세요
	// 적용 완료
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
	inline const char* RedisLoginKey(const char* userId)
	{
		return std::string{ std::string{"Login/"} + userId }.c_str();
	}
}