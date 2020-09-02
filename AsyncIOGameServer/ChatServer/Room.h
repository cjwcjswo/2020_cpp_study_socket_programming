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
	// TODO 최흥배  일부러 deque를 사용한 것 같은데 이유가 무엇인가요? 성능이나 추상화 측면에서 올바르지 않습니다.
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