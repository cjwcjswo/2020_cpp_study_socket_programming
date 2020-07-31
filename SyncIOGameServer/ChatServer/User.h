#pragma once

#include "../../NetworkLib/PrimitiveTypes.h"


enum class UserState : uint8
{
	DISCONNECT,
	CONNECT,
	LOGIN,
};

class User
{
public:
	int32 mSessionIndex = -1;
	uint64 mSessionUniqueId = 0;

	int32 mIndex = -1;
	uint64 mUid = 0;

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

