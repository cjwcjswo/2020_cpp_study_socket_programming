#pragma once

#include "../../NetworkLib/PrimitiveTypes.h"

class User
{
public:
	int32 mSessionIndex = -1;
	uint64 mSessionUniqueId = 0;

	int32 mIndex = -1;
	uint64 mUid = 0;


public:
	User() = default;
	explicit User(const int32 sessionIndex, const uint64 sessionUniqueId, const uint64 uid) : mSessionIndex(sessionIndex), mSessionUniqueId(sessionUniqueId), mUid(uid) {};

	~User() = default;


public:
	void Connect(const User& User);
	void Clear();
};

