#include "ClientSessionManager.h"

#include "Define.h"
#include "ClientSession.h"
#include "SList.h"

using namespace NetworkLib;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ClientSessionManager::Init(const uint32 maxClientSessionNum, const uint32 maxSessionBufferSize) noexcept
{
	mMaxSessionSize = maxClientSessionNum;
	mMaxSessionBufferSize = maxSessionBufferSize;

	mClientVector.reserve(maxClientSessionNum);
	mClientIndexPool = new SList;
	if (!mClientIndexPool->Create(maxClientSessionNum))
	{
		return ErrorCode::CLIENT_SESSION_MANAGER_INIT_FAIL;
	}

	ErrorCode errorCode;
	for (uint32 i = 0; i < maxClientSessionNum; ++i)
	{
		IndexElement* indexElement = reinterpret_cast<IndexElement*>(_aligned_malloc(sizeof(IndexElement), MEMORY_ALLOCATION_ALIGNMENT));
		if (indexElement == nullptr)
		{
			return ErrorCode::CLIENT_SESSION_MANAGER_INDEX_ALIGN_FAIL;
		}
		indexElement->mIndex = static_cast<int32>(i);
		mClientIndexPool->Push(reinterpret_cast<SLIST_ENTRY*>(indexElement));

		TCPSocket* tcpSocket = new TCPSocket();
		errorCode = tcpSocket->Create();
		if (errorCode != ErrorCode::SUCCESS)
		{
			return errorCode;
		}

		//TODO 최흥배
		// 이렇게 사용하면 emplace_back의 효과는 얻을 수 없습니다.
		// 그냥 push_back을 사용하세요.
		// 특히 현재 포인터를 저장하는 것이라서 올바르게 emplace_back를 사용하더라도 별 이득이 없습니다
		mClientVector.emplace_back(i, 0, tcpSocket);
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSession* ClientSessionManager::FindClientSession(const int32 index)
{
	if (index < 0 || index > static_cast<int32>(mMaxSessionSize) - 1)
	{
		return nullptr;
	}

	return &mClientVector[index];
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint64  ClientSessionManager::GenerateUniqueId() const
{
	return ++mUniqueIdGenerator;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
int32 ClientSessionManager::AllocClientSessionIndex()
{
	if (mClientIndexPool->Count() == 0)
	{
		return INVALID_INDEX;
	}
	IndexElement* indexElement = reinterpret_cast<IndexElement*>(mClientIndexPool->Pop());

	return indexElement->mIndex;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSession& ClientSessionManager::ConnectClientSession(ClientSession& clientSession)
{
	ClientSession& session = mClientVector[clientSession.mIndex];
	session.mUniqueId = clientSession.mUniqueId;
	session.mTCPSocket = clientSession.mTCPSocket;
	session.mIsConnect = true;

	return session;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSessionManager::DisconnectClientSession(const int32 index)
{
	mClientVector[index].Clear();
	IndexElement indexElement;
	indexElement.mIndex = static_cast<int32>(index);
	mClientIndexPool->Push(reinterpret_cast<SLIST_ENTRY*>(&indexElement));
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSessionManager::FlushSendClientSessionAll()
{
	for (auto& session : mClientVector)
	{
		if (!session.IsConnect())
		{
			continue;
		}

		session.FlushSend();
	}
}
