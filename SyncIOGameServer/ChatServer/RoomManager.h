#pragma once

#include <deque>

#include "../../NetworkLib/Protocol.h"
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
	std::deque<Room> mRoomDeque;


public:
	void Init(const int32 maxRoomNum);
	Room* FindRoom(int32 roomIndex);
};

