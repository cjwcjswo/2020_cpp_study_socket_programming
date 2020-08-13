#include "User.h"


using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void User::Clear()
{
	mSessionIndex = INVALID_INDEX;
	mSessionUniqueId = INVALID_UNIQUE_ID;
	mIndex = INVALID_INDEX;
	mUserId = nullptr;
	mState = UserState::DISCONNECT;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode User::Connect(const User& user)
{
	if (mState != UserState::DISCONNECT)
	{
		return ErrorCode::USER_ALREADY_CONNECTED_STATE;
	}

	mSessionIndex = user.mSessionIndex;
	mSessionUniqueId = user.mSessionUniqueId;
	mIndex = user.mIndex;
	mState = UserState::CONNECT;

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode User::Login(const char* userId)
{
	if (mState == UserState::DISCONNECT)
	{
		return ErrorCode::USER_NOT_CONNECTED_STATE;
	}

	if (mState == UserState::LOGIN || mState == UserState::ROOM)
	{
		return ErrorCode::USER_ALREADY_LOGIN_STATE;
	}

	mUserId = userId;
	mState = UserState::LOGIN;

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode User::EnterRoom(int32 roomIndex)
{
	if (mState == UserState::DISCONNECT || mState == UserState::CONNECT)
	{
		return ErrorCode::USER_NOT_LOGIN_STATE;
	}

	if (mState == UserState::ROOM)
	{
		return ErrorCode::USER_ALREADY_IN_THE_ROOM;
	}

	mRoomIndex = roomIndex;
	mState = UserState::ROOM;

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode User::LeaveRoom()
{
	if (mState != UserState::ROOM)
	{
		return ErrorCode::USER_NOT_ROOM_ENTER_STATE;
	}

	mRoomIndex = INVALID_INDEX;
	mState = UserState::LOGIN;

	return ErrorCode::SUCCESS;
}