#include <iostream>
#include "ClientSessionManager.h"
#include "ClientSession.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSessionManager::Init(const int maxClientSessionNum) noexcept
{
	mMaxSessionSize = maxClientSessionNum;
	mClientVector.reserve(maxClientSessionNum);

	for (int i = 0; i < maxClientSessionNum; ++i)
	{
		mClientIndexPool.push(i);
		mClientVector.emplace_back(-1, 0, INVALID_SOCKET);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint64  ClientSessionManager::GenerateUniqueId() const
{
	return ++mUniqueIdGenerator;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int32 ClientSessionManager::AllocClientSessionIndex()
{
	if (mClientIndexPool.empty())
	{
		return ClientSession::INVALID_INDEX;
	}
	int32 index = mClientIndexPool.front();
	mClientIndexPool.pop();

	return index;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSessionManager::ConnectClientSession(ClientSession& clientSession)
{
	mClientVector[clientSession.mIndex] = clientSession;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSessionManager::DisconnectClientSession(const int32 clientIndex)
{
	mClientVector[clientIndex].Clear();
	mClientIndexPool.push(clientIndex);
}

void ClientSessionManager::DisconnectClientSession(const uint64 clientUniqueId)
{
	for (int i = 0; i < mMaxSessionSize; i++)
	{
		if (mClientVector[i].mUniqueId == clientUniqueId)
		{
			mClientVector[i].Clear();
			mClientIndexPool.push(i);
			return;
		}
	}
}

void ClientSessionManager::DisconnectClientSession(const SOCKET clientSocket)
{
	for (int i = 0; i < mMaxSessionSize; i++)
	{
		if (mClientVector[i].mSocket == clientSocket)
		{
			mClientVector[i].Clear();
			mClientIndexPool.push(i);
			return;
		}
	}
}