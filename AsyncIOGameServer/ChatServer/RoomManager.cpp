#include "RoomManager.h"

#include "Room.h"
#include "User.h"


using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Room* RoomManager::FindRoom(const int32 roomIndex)
{
	if (roomIndex > static_cast<int32>(mMaxRoomNum) - 1)
	{
		return nullptr;
	}

	return &mRoomList[roomIndex];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RoomManager::Init(const int32 maxRoomNum, const uint32 maxRoomUserNum)
{
	mMaxRoomNum = maxRoomNum;
	mRoomList = new Room[maxRoomNum];
	
	for (int32 roomIndex = 0; roomIndex < maxRoomNum; ++roomIndex)
	{
		mRoomList[roomIndex].Init(roomIndex, maxRoomUserNum);
	}
}