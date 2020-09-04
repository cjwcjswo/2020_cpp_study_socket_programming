#include "Room.h"
#include "User.h"


using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
User* Room::FindUser(const uint64 sessionUniqueId)
{
	for (int i = 0; i < mMaxRoomUserNum; ++i)
	{
		if (mRoomUserList[i] == nullptr)
		{
			continue;
		}

		if (mRoomUserList[i]->mSessionUniqueId == sessionUniqueId)
		{
			return mRoomUserList[i];
		}
	}
	
	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Room::Init(const int32 roomIndex, const uint16 maxUserNum)
{
	mRoomIndex = roomIndex;
	mMaxRoomUserNum = maxUserNum;

	mRoomUserList = new User*[maxUserNum];
	for (int i = 0; i < maxUserNum; ++i)
	{
		mRoomUserIndexPool.push(i);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Room::Enter(User& user)
{
	if ((mMaxRoomUserNum <= mRoomUserCount) || mRoomUserIndexPool.empty())
	{
		return ErrorCode::ROOM_IS_FULL;
	}

	User* findUser = FindUser(user.mSessionUniqueId);
	if (findUser != nullptr)
	{
		return ErrorCode::USER_ALREADY_IN_THE_ROOM;
	}

	ErrorCode errorCode = user.EnterRoom(mRoomIndex);
	if (errorCode != ErrorCode::SUCCESS)
	{
		return errorCode;
	}

	int32 roomUserIndex = mRoomUserIndexPool.front();
	mRoomUserIndexPool.pop();
	mRoomUserList[roomUserIndex] = &user;

	++mRoomUserCount;

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Room::Leave(User& user)
{
	uint64 sessionUniqueId = INVALID_UNIQUE_ID;
	for (int i = 0; i < mMaxRoomUserNum; ++i)
	{
		if (mRoomUserList[i] == nullptr)
		{
			continue;
		}

		if (mRoomUserList[i]->mSessionUniqueId == sessionUniqueId)
		{
			mRoomUserIndexPool.push(i);
			return user.LeaveRoom();
		}
	}

	return ErrorCode::SUCCESS;
}