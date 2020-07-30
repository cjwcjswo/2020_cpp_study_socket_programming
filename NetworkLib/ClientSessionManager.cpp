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
ClientSession* ClientSessionManager::FindClientSession(const int32 index)
{
	ClientSession* session = &mClientVector[index];
	if (!session->IsConnect())
	{
		return nullptr;
	}

	return session;
}

ClientSession* ClientSessionManager::FindClientSession(const uint64 uniqueId)
{
	
	for (int i = 0; i < mMaxSessionSize; i++)
	{
		if (mClientVector[i].mUniqueId == uniqueId)
		{
			return &mClientVector[i];
		}
	}

	return nullptr;
}

ClientSession* ClientSessionManager::FindClientSession(const SOCKET socket)
{
	for (int i = 0; i < mMaxSessionSize; i++)
	{
		if (mClientVector[i].mSocket == socket)
		{
			return &mClientVector[i];
		}
	}

	return nullptr;
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
	ClientSession* session = FindClientSession(clientUniqueId);
	if (nullptr == session)
	{
		return;
	}

	mClientIndexPool.push(session->mIndex);
	session->Clear();
}

void ClientSessionManager::DisconnectClientSession(const SOCKET clientSocket)
{
	ClientSession* session = FindClientSession(clientSocket);
	if (nullptr == session)
	{
		return;
	}

	mClientIndexPool.push(session->mIndex);
	session->Clear();
}