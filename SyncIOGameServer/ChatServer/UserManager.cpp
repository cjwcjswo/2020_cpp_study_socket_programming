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

	mUserPool[userIndex].Connect(user);

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CS::ErrorCode UserManager::Disconnect(const int32 userIndex)
{
	mUserPool[userIndex].Clear();
	mUserIndexPool.push(userIndex);

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CS::ErrorCode UserManager::Login(const uint64 sessionUniqueId, const uint64 uid)
{
	User* user = FindUser(sessionUniqueId);
	if (nullptr == user)
	{
		return ErrorCode::INVALID_USER;
	}
	user->Login(uid);

	return ErrorCode::SUCCESS;
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