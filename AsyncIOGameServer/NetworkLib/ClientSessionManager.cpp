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
	mClientIndexPool = new SList<IndexElement>;
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
		mClientIndexPool->Push(indexElement);

		TCPSocket* tcpSocket = new TCPSocket();
		errorCode = tcpSocket->Create();
		if (errorCode != ErrorCode::SUCCESS)
		{
			return errorCode;
		}

		mClientVector.emplace_back(indexElement, 0, tcpSocket);
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

//TODO 최흥배
// 아램 함수로는 검색하지 않도록 합니다. 전체 검색이 너무 되네요
// FindClientSession(const int32 index)로만 검색하고, 검증이 필요하면 index로 검색 후 uniqueId로 확인하면 될 것 같습니다
// 적용 완료(함수 삭제)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint64  ClientSessionManager::GenerateUniqueId() const
{
	return ++mUniqueIdGenerator;
}

//TODO 최흥배
// 멀티스레드에서 호출되는데 스레드 세이프 하지 않습니다.
// Interlocked Singly linked list를 사용해보죠
// 적용 완료
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
IndexElement* ClientSessionManager::AllocClientSessionIndexElement()
{
	if (mClientIndexPool->Count() == 0)
	{
		return nullptr;
	}

	return mClientIndexPool->Pop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSession& ClientSessionManager::ConnectClientSession(ClientSession& clientSession)
{
	ClientSession& session = mClientVector[clientSession.mIndexElement->mIndex];
	session.mUniqueId = clientSession.mUniqueId;
	session.mTCPSocket = clientSession.mTCPSocket;
	session.mIsConnect = true;

	return session;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSessionManager::DisconnectClientSession(const int32 index)
{
	mClientIndexPool->Push(mClientVector[index].mIndexElement);
	mClientVector[index].Clear();
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
