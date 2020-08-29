﻿#include "ClientSessionManager.h"

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

//TODO �����
// �Ʒ� �Լ��δ� �˻����� �ʵ��� �մϴ�. ��ü �˻��� �ʹ� �ǳ׿�
// FindClientSession(const int32 index)�θ� �˻��ϰ�, ������ �ʿ��ϸ� index�� �˻� �� uniqueId�� Ȯ���ϸ� �� �� �����ϴ�
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
ClientSession* ClientSessionManager::FindClientSessionBySocket(const SOCKET socket)
{
	for (uint32 i = 0; i < mMaxSessionSize; i++)
	{
		if (mClientVector[i].mTCPSocket->mSocket == socket)
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

//TODO �����
// ��Ƽ�����忡�� ȣ��Ǵµ� ������ ������ ���� �ʽ��ϴ�.
// Interlocked Singly linked list�� ����غ���
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
