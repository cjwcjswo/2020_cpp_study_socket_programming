#pragma once

#include <vector>
#include <queue>

#include "../../NetworkLib/PrimitiveTypes.h"
#include "User.h"
#include "ErrorCode.h"


class UserManager
{
private:
	std::vector<User> mUserPool;
	std::queue<int32> mUserIndexPool;

	int mMaxUserNum = 0;


public:
	UserManager() = default;
	~UserManager() = default;


private:
	int32 AllocUserIndex();

	
public:
	CS::ErrorCode Init(const int maxUserNum);

	CS::ErrorCode Connect(User& user);

	CS::ErrorCode Login(const uint64 sessionUniqueId, const char* userId);

	CS::ErrorCode Disconnect(const int32 userIndex);

	UserState UserState(const uint64 sessionUniqueId);

	User* FindUser(const uint64 sessionUniqueId);
};

