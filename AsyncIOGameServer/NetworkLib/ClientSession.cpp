﻿#pragma comment(lib, "mswsock.lib")  

#include "ClientSession.h"

#include <WinSock2.h>
#include <mswsock.h>
#include "OverlappedIOContext.h"


using namespace NetworkLib;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSession::ClientSession(const int32 index, const uint64 uniqueId, TCPSocket* tcpSocket) : 
	mIndex(index), mUniqueId(uniqueId), mTCPSocket(tcpSocket), mSendBuffer(1024), mReceiveBuffer(1024)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSession::~ClientSession()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ClientSession::IsConnect() const
{
	if (mTCPSocket == nullptr)
	{
		return false;
	}

	if (mTCPSocket->mSocket == INVALID_SOCKET)
	{
		return false;
	}

	return mIsConnect;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ClientSession::ReceiveAsync()
{
	if (!IsConnect())
	{
		return ErrorCode::CLIENT_SESSION_NOT_CONNECTED;
	}

	FastSpinlockGuard criticalSection(mSessionLock);

	if (mReceiveBuffer.RemainBufferSize() == 0)
	{
		return ErrorCode::CLIENT_SESSION_RECEIVE_BUFFER_FULL;
	}

	OverlappedIOReceiveContext* context = new OverlappedIOReceiveContext(this);

	DWORD receiveBytes = 0;
	DWORD flags = 0;
	context->mWSABuf.buf = mReceiveBuffer.GetBuffer();
	context->mWSABuf.len = static_cast<ULONG>(mReceiveBuffer.RemainBufferSize());

	if (WSARecv(mTCPSocket->mSocket, &context->mWSABuf, 1, &receiveBytes, &flags, (LPWSAOVERLAPPED)context, nullptr) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIOContext(context);
			return ErrorCode::CLIENT_SESSION_RECEIVE_FAIL;
		}
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSession::ReceiveCompletion(DWORD transferred)
{
	FastSpinlockGuard criticalSection(mSessionLock);

	mReceiveBuffer.Commit(transferred);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ClientSession::SendAsync(const char* data, size_t length)
{
	FastSpinlockGuard criticalSection(mSessionLock);

	if (!IsConnect())
	{
		return ErrorCode::CLIENT_SESSION_NOT_CONNECTED;
	}

	if (!mSendBuffer.Push(data, length))
	{
		return ErrorCode::CLIENT_SESSION_SEND_FAIL;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ClientSession::FlushSend()
{
	if (!IsConnect())
	{
		return ErrorCode::CLIENT_SESSION_NOT_CONNECTED;
	}

	FastSpinlockGuard criticalSection(mSessionLock);

	if (mSendBuffer.DataSize() == 0 || mSendPendingCount > 0)
	{
		return ErrorCode::SUCCESS;
	}

	if (mSendBuffer.RemainBufferSize() == 0)
	{
		return ErrorCode::CLIENT_SESSION_SEND_FAIL;
	}

	OverlappedIOSendContext* context = new OverlappedIOSendContext(this);

	DWORD snedBytes = 0;
	DWORD flags = 0;
	context->mWSABuf.len = static_cast<ULONG>(mSendBuffer.DataSize());
	context->mWSABuf.buf = mSendBuffer.FrontData();

	if (WSASend(mTCPSocket->mSocket, &context->mWSABuf, 1, &snedBytes, flags, (LPWSAOVERLAPPED)context, nullptr) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			DeleteIOContext(context);
			return ErrorCode::CLIENT_SESSION_SEND_FAIL;
		}
	}

	++mSendPendingCount;

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSession::SendCompletion(DWORD transferred)
{
	FastSpinlockGuard criticalSection(mSessionLock);

	if (!IsConnect())
	{
		return;
	}

	mSendBuffer.Pop(transferred);

	--mSendPendingCount;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkLib::ClientSession::DisconnectAsync()
{
	// TODO: 집 운영체제가 윈도우 7이라 DisconnectEx를 지원 안함 -.-;;... 운영체제별 분기 처리
	closesocket(mTCPSocket->mSocket);
	mTCPSocket->Clear();
	mTCPSocket->Create();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkLib::ClientSession::DisconnectCompletion()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSession::Clear()
{
	mIndex = -1;
	mUniqueId = 0;
	mSendPendingCount = 0;
	mTCPSocket->Clear();
}