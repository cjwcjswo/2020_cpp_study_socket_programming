#pragma once

#include <deque>

#include "../NetworkLib/Protocol.h"
#include "Protocol.h"
#include "ErrorCode.h"
#include "Room.h"


class User;

class RoomManager
{
public:
	RoomManager() = default;
	~RoomManager() = default;


private:
	// TODO 최흥배
	// 적합하지 않은 자료구조를 사용했습니다
	// 적용 완료
	Room* mRoomList = nullptr;
	uint32 mMaxRoomNum = 0;


public:
	void Init(const int32 maxRoomNum, const uint32 maxRoomUserNum);
	Room* FindRoom(const int32 roomIndex);
};

