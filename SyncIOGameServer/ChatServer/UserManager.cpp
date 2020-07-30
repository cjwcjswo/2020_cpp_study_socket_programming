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
		return -1;
	}

	int32 index = mUserIndexPool.front();
	mUserIndexPool.pop();

	return index;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode UserManager::Connect(User& user)
{
	int32 userIndex = AllocUserIndex();
	if (-1 == userIndex)
	{
		return ErrorCode::USER_MANAGER_POOL_IS_FULL;
	}
	
	user.mIndex = userIndex;
	mUserPool[userIndex] = user;

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CS::ErrorCode UserManager::Disconnect(User& user)
{
	mUserPool[user.mIndex].Clear();
	mUserIndexPool.push(user.mIndex);

	return ErrorCode::SUCCESS;
}