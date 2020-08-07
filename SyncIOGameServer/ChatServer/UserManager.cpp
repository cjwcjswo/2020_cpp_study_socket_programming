#include "UserManager.h"
#include "User.h"


using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode UserManager::Init(const int maxUserNum)
{
	mUserPool.reserve(maxUserNum);
	for (int i = 0; i < maxUserNum; i++)
	{
		mUserIndexPool.push(i);
		mUserPool.emplace_back();
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int32 UserManager::AllocUserIndex()
{
	if (mUserIndexPool.empty())
	{
		return INVALID_INDEX;
	}

	int32 index = mUserIndexPool.front();
	mUserIndexPool.pop();

	return index;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode UserManager::Connect(User& user)
{
	int32 userIndex = AllocUserIndex();
	if (INVALID_INDEX == userIndex)
	{
		return ErrorCode::USER_MANAGER_POOL_IS_FULL;
	}
	
	user.mIndex = userIndex;

	return mUserPool[userIndex].Connect(user);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CS::ErrorCode UserManager::Disconnect(const int32 userIndex)
{
	mUserPool[userIndex].Clear();
	mUserIndexPool.push(userIndex);

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CS::ErrorCode UserManager::Login(const uint64 sessionUniqueId, const char* userId)
{
	User* user = FindUser(sessionUniqueId);
	if (nullptr == user)
	{
		return ErrorCode::USER_IS_INVALID;
	}

	return user->Login(userId);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
User* UserManager::FindUser(const uint64 sessionUniqueId)
{
	auto userIter = std::find_if(mUserPool.begin(), mUserPool.end(), [sessionUniqueId](User& user) {return user.mSessionUniqueId == sessionUniqueId; });
	if (userIter == mUserPool.end())
	{
		return nullptr;
	}

	return &(*userIter);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
UserState UserManager::UserState(const uint64 sessionUniqueId)
{
	User* user = FindUser(sessionUniqueId);
	if (nullptr == user)
	{
		return UserState::DISCONNECT;
	}

	return user->mState;
}