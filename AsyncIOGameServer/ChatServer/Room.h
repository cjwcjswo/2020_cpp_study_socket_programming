#pragma once

#include <queue>

#include "../NetworkLib/Define.h"
#include "ErrorCode.h"
#include "User.h"


class Room
{
public:
	Room() = default;
	~Room() = default;


private:
	int32 mRoomIndex = INVALID_INDEX;
	uint16 mMaxRoomUserNum = 0;
	std::queue<int32> mRoomUserIndexPool;


public:
	// TODO 최흥배  일부러 deque를 사용한 것 같은데 이유가 무엇인가요? 성능이나 추상화 측면에서 올바르지 않습니다.
	// 적용 완료
	User** mRoomUserList = nullptr;
	uint16 mRoomUserCount = 0;


private:
	User* FindUser(const uint64 sessionUniqueId);


public:
	void Init(const int32 roomIndex, const uint16 maxUserNum);
	CS::ErrorCode Enter(User& user);
	CS::ErrorCode Leave(User& user);
};