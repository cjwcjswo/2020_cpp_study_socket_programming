#pragma comment(lib, "mswsock.lib")  

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
	// TODO 최흥배
	// mReceiveBuffer 이 버퍼를 이 세션 객체만 접근하고 recv는 한번에 한번씩만 하기 때문에 락을 걸 필요가 없습니다.
	// 만약 패킷을 처리하는 스레드에서 이 버퍼를 접근한다면 당연 락을 걸어야 하지만 지금은 접근하지 않는걸로 보입니다.
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

	// TODO 최흥배
	// mSendPendingCount 1과 0 두가지 값만 가지니 bool 타입이 좋을 것 같아요
	// mSendPendingCount의 타입과 이름을 보면 send 횟수를 계속 카운팅해서 뭔가에 사용할 것 같은데 지금 코드에서느 그렇지 않네요
	if (mSendBuffer.DataSize() == 0 || mSendPendingCount > 0)
	{
		return ErrorCode::SUCCESS;
	}

	if (mSendBuffer.RemainBufferSize() == 0)
	{
		return ErrorCode::CLIENT_SESSION_SEND_FAIL;
	}

	OverlappedIOSendContext* context = new OverlappedIOSendContext(this);


	// TODO 최흥배
	// 보낼 때 MSS 사이즈를 넘지 않게 보내도록 하죠
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
	// TODO 최흥배
	// DisconnectEx 사용은 개인적으로 비추입니다.
	// DisconnectEx를 사용하면 OS TIME_WAIT의 동작을 생각하고 코딩해야 합니다

	// mTCPSocket 객체의 멤버를 멀티스레드에서 접근해서 아래처럼 하면 문제가 있지 않을까요?

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