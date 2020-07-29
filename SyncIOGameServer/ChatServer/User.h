#pragma once

#include "../../NetworkLib/PrimitiveTypes.h"

class User
{
public:
	User();
	~User() = default;


private:
	int32 mSessionIndex = -1;
	uint64 mSessionUniqueId = 0;

	int32 mIndex = -1;
	uint64 mUid = 0;


public:
	void Clear();
};

