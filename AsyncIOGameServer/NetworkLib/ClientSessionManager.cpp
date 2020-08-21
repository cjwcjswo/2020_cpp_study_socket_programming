#pragma comment(lib,"ws2_32")

#include <WinSock2.h>

#include "Define.h"
#include "ClientSessionManager.h"
#include "ClientSession.h"


using namespace NetworkLib;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ClientSessionManager::Init(const uint32 maxClientSessionNum, const uint32 maxSessionBufferSize) noexcept
{
	mMaxSessionSize = maxClientSessionNum;
	mMaxSessionBufferSize = maxSessionBufferSize;

	mClientVector.reserve(maxClientSessionNum);

	ErrorCode errorCode;
	for (uint32 i = 0; i < maxClientSessionNum; ++i)
	{
		mClientIndexPool.push(i);

		TCPSocket* tcpSocket = new TCPSocket();
		errorCode = tcpSocket->Create();
		if (errorCode != ErrorCode::SUCCESS)
		{
			return errorCode;
		}

		mClientVector.emplace_back(i, 0, tcpSocket);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSession* ClientSessionManager::FindClientSession(const int32 index)
{
	if (index < 0 || index > mMaxSessionSize - 1)
	{
		return nullptr;
	}

	return &mClientVector[index];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
		return INVALID_INDEX;
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
	session.mTCPSocket = clientSession.mTCPSocket;
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
	if (session == nullptr)
	{
		return;
	}

	mClientIndexPool.push(session->mIndex);
	session->Clear();
}