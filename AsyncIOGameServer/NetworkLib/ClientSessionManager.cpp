#include "ClientSessionManager.h"

#include "ClientSession.h"
#include "OverlappedIOContext.h"


using namespace NetworkLib;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ClientSessionManager::Init(const uint32 maxClientSessionNum, const uint32 maxSessionBufferSize, const uint32 socketAddressBufferSize, const uint32 spinLockCount) noexcept
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

		TCPSocket* tcpSocket = new TCPSocket(socketAddressBufferSize);
		errorCode = tcpSocket->Create();
		if (errorCode != ErrorCode::SUCCESS)
		{
			return errorCode;
		}

		//TODO 최흥배
		// 이렇게 사용하면 emplace_back의 효과는 얻을 수 없습니다.
		// 그냥 push_back을 사용하세요.
		// 특히 현재 포인터를 저장하는 것이라서 올바르게 emplace_back를 사용하더라도 별 이득이 없습니다
		// 적용 완료
		mClientVector.push_back(new ClientSession(indexElement, 0, tcpSocket, spinLockCount, maxSessionBufferSize));
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

	return mClientVector[index];
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const uint64 ClientSessionManager::GenerateUniqueId()
{
	return mUniqueIdGenerator++;;
}


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
ErrorCode ClientSessionManager::ConnectClientSession(HANDLE iocpHandle, TCPSocket* tcpSocket)
{
	IndexElement* indexElement = AllocClientSessionIndexElement();
	if (indexElement == nullptr)
	{
		return ErrorCode::CLIENT_SESSION_MANAGER_INDEX_ALIGN_FAIL;
	}

	ClientSession* session = mClientVector[indexElement->mIndex];
	session->mUniqueId = GenerateUniqueId();
	session->mTCPSocket = tcpSocket;
	session->mIsConnect = true;

	CreateIoCompletionPort((HANDLE)tcpSocket->mSocket, iocpHandle, static_cast<ULONG_PTR>(IOKey::RECEIVE), 0);
	session->ReceiveAsync();

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSessionManager::DisconnectClientSession(const int32 index)
{
	mClientIndexPool->Push(mClientVector[index]->mIndexElement);
	mClientVector[index]->Clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSessionManager::FlushSendClientSessionAll()
{
	for (auto& session : mClientVector)
	{
		if (!session->IsConnect())
		{
			continue;
		}

		session->FlushSend();
	}
}
