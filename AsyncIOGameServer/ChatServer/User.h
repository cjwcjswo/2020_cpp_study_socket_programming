#pragma once

#include "../NetworkLib/PrimitiveTypes.h"
#include "../NetworkLib/ClientSession.h"
#include "../NetworkLib/Define.h"

#include "ErrorCode.h"


enum class UserState : uint8
{
	DISCONNECT,
	CONNECT,
	LOGIN,
	ROOM,
};


class User
{
public:
	int32 mSessionIndex = INVALID_INDEX;
	uint64 mSessionUniqueId = INVALID_UNIQUE_ID;

	int32 mIndex = INVALID_INDEX;
	const char* mUserId = nullptr;

	int mRoomIndex = INVALID_INDEX;

	UserState mState = UserState::DISCONNECT;


public:
	User() = default;
	explicit User(const int32 sessionIndex, const uint64 sessionUniqueId) : mSessionIndex(sessionIndex), mSessionUniqueId(sessionUniqueId) {};

	~User() = default;


public:
	void Clear();
	CS::ErrorCode Connect(const User& User);
	CS::ErrorCode Login(const char* mUserId);
	CS::ErrorCode EnterRoom(const int32 roomIndex);
	CS::ErrorCode LeaveRoom();
};