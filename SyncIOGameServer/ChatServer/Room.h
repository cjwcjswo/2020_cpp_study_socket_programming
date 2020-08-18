#pragma once

#include <deque>

#include "../NetworkLib/Define.h"
#include "ErrorCode.h"
#include "User.h"


class Room
{
public:
	explicit Room(const int32 roomIndex, const uint16 maxUserNum) : mRoomIndex(roomIndex), mMaxUserNum(maxUserNum) {};
	~Room() = default;


private:
	std::deque<User> mUserDeque;
	int32 mRoomIndex = INVALID_INDEX;
	uint16 mMaxUserNum = 0;


private:
	User* FindUser(const uint64 sessionUniqueId);


public:
	CS::ErrorCode Enter(User& user);
	CS::ErrorCode Leave(User& user);

	inline std::deque<User>& UserDeque() { return mUserDeque; }
};

