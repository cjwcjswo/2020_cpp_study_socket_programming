#include "RoomManager.h"
#include "Room.h"
#include "User.h"


using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Room* RoomManager::FindRoom(const int32 roomIndex)
{
	if (roomIndex > (mRoomDeque.size() - 1))
	{
		return nullptr;
	}

	return &mRoomDeque[roomIndex];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RoomManager::Init(const int32 maxRoomNum)
{
	for (int32 roomIndex = 0; roomIndex < maxRoomNum; ++roomIndex)
	{
		mRoomDeque.emplace_back(roomIndex, 20);
	}
}