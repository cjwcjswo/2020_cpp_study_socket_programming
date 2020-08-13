#include "Room.h"
#include "User.h"


using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
User* Room::FindUser(uint64 sessionUniqueId)
{
	auto userIter = std::find_if(mUserDeque.begin(), mUserDeque.end(), [sessionUniqueId](User& user) {return user.mSessionUniqueId == sessionUniqueId; });
	if (userIter == mUserDeque.end())
	{
		return nullptr;
	}

	return &(*userIter);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Room::Enter(User& user)
{
	if (mMaxUserNum <= mUserDeque.size())
	{
		return ErrorCode::ROOM_IS_FULL;
	}

	User* findUser = FindUser(user.mSessionUniqueId);
	if (nullptr != findUser)
	{
		return ErrorCode::USER_ALREADY_IN_THE_ROOM;
	}

	ErrorCode errorCode = user.EnterRoom(mRoomIndex);
	if (errorCode != ErrorCode::SUCCESS)
	{
		return errorCode;
	}

	mUserDeque.push_back(user);

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Room::Leave(User& user)
{
	uint64 userSessionUniqueId = user.mSessionUniqueId;

	auto userIter = std::find_if(mUserDeque.begin(), mUserDeque.end(), [userSessionUniqueId](User& user) {return user.mSessionUniqueId == userSessionUniqueId; });
	if (userIter != mUserDeque.end())
	{
		mUserDeque.erase(userIter);
	}

    return user.LeaveRoom();
}