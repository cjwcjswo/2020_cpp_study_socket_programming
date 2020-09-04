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
		mUserPool.push_back(User{});
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
	if (userIndex == INVALID_INDEX)
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
	if (user == nullptr)
	{
		return ErrorCode::USER_IS_INVALID;
	}

	return user->Login(userId);
}

// TODO 최흥배
// 검색 비용이 발생하지 않도록 sessionUniqueId 검색이 아닌 userIndex로 검색하도록 합니다.
// sessionUniqueId는 이후 정확한지 한번 더 체크하는 용도로 비교하면 좋을 것 같습니다.
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
	if (user == nullptr)
	{
		return UserState::DISCONNECT;
	}

	return user->mState;
}