#pragma comment(lib,"ws2_32")

#include <WinSock2.h>
#include "ClientSessionManager.h"
#include "ClientSession.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSessionManager::Init(const uint32 maxClientSessionNum, const  uint32 maxSessionBufferSize) noexcept
{
	mMaxSessionSize = maxClientSessionNum;
	mMaxSessionBufferSize = maxSessionBufferSize;

	mClientVector.reserve(maxClientSessionNum);

	for (uint32 i = 0; i < maxClientSessionNum; ++i)
	{
		mClientIndexPool.push(i);
		mClientVector.emplace_back(i, 0, INVALID_SOCKET, maxSessionBufferSize);
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
	
	for (uint32 i = 0; i < mMaxSessionSize; i++)
	{
		if (mClientVector[i].mUniqueId == uniqueId)
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
	ClientSession& session = mClientVector[clientSession.mIndex];
	session.mUniqueId = clientSession.mUniqueId;
	session.mSocket = clientSession.mSocket;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSessionManager::DisconnectClientSession(const int32 index)
{
	mClientVector[index].Clear();
	mClientIndexPool.push(index);
}

void ClientSessionManager::DisconnectClientSession(const uint64 uniqueId)
{
	ClientSession* session = FindClientSession(uniqueId);
	if (nullptr == session)
	{
		return;
	}

	mClientIndexPool.push(session->mIndex);
	session->Clear();
}