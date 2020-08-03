#pragma once

#include "../../NetworkLib/PrimitiveTypes.h"
#include "../../NetworkLib/ClientSession.h"


enum class UserState : uint8
{
	DISCONNECT,
	CONNECT,
	LOGIN,
};


class User
{
public:
	constexpr static int32 INVALID_USER_INDEX = -1;
	constexpr static uint64 INVALID_UID = 0;


public:
	int32 mSessionIndex = ClientSession::INVALID_INDEX;
	uint64 mSessionUniqueId = ClientSession::INVALID_UNIQUE_ID;

	int32 mIndex = INVALID_USER_INDEX;
	uint64 mUid = INVALID_UID;

	UserState mState = UserState::DISCONNECT;


public:
	User() = default;
	explicit User(const int32 sessionIndex, const uint64 sessionUniqueId) : mSessionIndex(sessionIndex), mSessionUniqueId(sessionUniqueId) {};

	~User() = default;


public:
	void Clear();
	void Connect(const User& User);
	void Login(uint64 uid);
};

