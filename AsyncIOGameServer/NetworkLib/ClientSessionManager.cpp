#include "ClientSessionManager.h"

#include "Define.h"
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
// 적용 완료
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
uint64  ClientSessionManager::GenerateUniqueId() const
{
	return ++mUniqueIdGenerator;
}

//TODO 최흥배
// 멀티스레드에서 호출되는데 스레드 세이프 하지 않습니다.
// Interlocked Singly linked list를 사용해보죠
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
