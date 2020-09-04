#pragma comment(lib, "mswsock.lib")  

#include "ClientSession.h"

#include <WinSock2.h>
#include <mswsock.h>
#include "OverlappedIOContext.h"


using namespace NetworkLib;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSession::ClientSession(IndexElement* indexElement, const uint64 uniqueId, TCPSocket* tcpSocket, const uint32 spinLockCount, const uint32 maxBufferSize) : 
	mIndexElement(indexElement), mUniqueId(uniqueId), mTCPSocket(tcpSocket), mSpinLockCount(spinLockCount), mSendBuffer(maxBufferSize), mReceiveBuffer(maxBufferSize)
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

	if (!InitializeCriticalSectionAndSpinCount(&mCriticalSection, mSpinLockCount))
	{
		return ErrorCode::CLIENT_SESSION_LOCK_FAIL;
	}

	if (mReceiveBuffer.RemainBufferSize() == 0)
	{
		DeleteCriticalSection(&mCriticalSection);

		return ErrorCode::CLIENT_SESSION_RECEIVE_BUFFER_FULL;
	}

	OverlappedIOContext* context = GIOContextPool->Pop();
	if (context == nullptr)
	{
		return ErrorCode::IO_CONTEXT_ELEMENT_IS_NOT_ENOUGH;
	}
	context->mIOKey = IOKey::RECEIVE;

	DWORD receiveBytes = 0;
	DWORD flags = 0;
	context->mWSABuf.buf = mReceiveBuffer.GetBuffer();
	context->mWSABuf.len = static_cast<ULONG>(mReceiveBuffer.RemainBufferSize());

	if (WSARecv(mTCPSocket->mSocket, &context->mWSABuf, 1, &receiveBytes, &flags, (LPWSAOVERLAPPED)context, nullptr) == SOCKET_ERROR)
	{
		if (WSAGetLastError() != WSA_IO_PENDING)
		{
			context->Clear();
			GIOContextPool->Push(context);
			DeleteCriticalSection(&mCriticalSection);

			return ErrorCode::CLIENT_SESSION_RECEIVE_FAIL;
		}
	}

	DeleteCriticalSection(&mCriticalSection);

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSession::ReceiveCompletion(DWORD transferred)
{
	// TODO 최흥배
	// mReceiveBuffer 이 버퍼를 이 세션 객체만 접근하고 recv는 한번에 한번씩만 하기 때문에 락을 걸 필요가 없습니다.
	// 만약 패킷을 처리하는 스레드에서 이 버퍼를 접근한다면 당연 락을 걸어야 하지만 지금은 접근하지 않는걸로 보입니다.
	// 적용 완료
	mReceiveBuffer.Commit(transferred);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ClientSession::SendAsync(const char* data, size_t length)
{
	if (!InitializeCriticalSectionAndSpinCount(&mCriticalSection, mSpinLockCount))
	{
		return ErrorCode::CLIENT_SESSION_LOCK_FAIL;
	}

	if (!IsConnect())
	{
		DeleteCriticalSection(&mCriticalSection);

		return ErrorCode::CLIENT_SESSION_NOT_CONNECTED;
	}

	if (!mSendBuffer.Push(data, length))
	{
		DeleteCriticalSection(&mCriticalSection);

		return ErrorCode::CLIENT_SESSION_SEND_FAIL;
	}

	DeleteCriticalSection(&mCriticalSection);


	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ClientSession::FlushSend()
{
	if (!IsConnect())
	{
		return ErrorCode::CLIENT_SESSION_NOT_CONNECTED;
	}

	if (!InitializeCriticalSectionAndSpinCount(&mCriticalSection, mSpinLockCount))
	{
		return ErrorCode::CLIENT_SESSION_LOCK_FAIL;
	}

	// TODO 최흥배
	// mSendPendingCount 1과 0 두가지 값만 가지니 bool 타입이 좋을 것 같아요
	// mSendPendingCount의 타입과 이름을 보면 send 횟수를 계속 카운팅해서 뭔가에 사용할 것 같은데 지금 코드에서느 그렇지 않네요
	// 적용 완료
	if (mSendBuffer.DataSize() == 0 || !isSending)
	{
		DeleteCriticalSection(&mCriticalSection);

		return ErrorCode::SUCCESS;
	}

	if (mSendBuffer.RemainBufferSize() == 0)
	{
		DeleteCriticalSection(&mCriticalSection);

		return ErrorCode::CLIENT_SESSION_SEND_FAIL;
	}

	// TODO 최흥배
	// 보낼 때 MSS 사이즈를 넘지 않게 보내도록 하죠
	// 적용 완료
	const int MAX_SEGMENT_SIZE = 1024; // 넉넉하게 1K
	ULONG remainSize = static_cast<ULONG>(mSendBuffer.DataSize());
	int pos = 0;

	while (remainSize > 0)
	{
		ULONG sendLength = MAX_SEGMENT_SIZE;
		if (remainSize < MAX_SEGMENT_SIZE)
		{
			sendLength = remainSize;
		}

		OverlappedIOContext* context = GIOContextPool->Pop();
		if (context == nullptr)
		{
			return ErrorCode::IO_CONTEXT_ELEMENT_IS_NOT_ENOUGH;
		}

		DWORD sendBytes = 0;
		DWORD flags = 0;
		context->mWSABuf.len = sendLength;
		context->mWSABuf.buf = mSendBuffer.FrontData() + pos;

		if (WSASend(mTCPSocket->mSocket, &context->mWSABuf, 1, &sendBytes, flags, (LPWSAOVERLAPPED)context, nullptr) == SOCKET_ERROR)
		{
			if (WSAGetLastError() != WSA_IO_PENDING)
			{
				DeleteCriticalSection(&mCriticalSection);
				context->Clear();
				GIOContextPool->Push(context);

				return ErrorCode::CLIENT_SESSION_SEND_FAIL;
			}
		}

		remainSize -= MAX_SEGMENT_SIZE;
		pos += MAX_SEGMENT_SIZE;
	}

	isSending = true;

	DeleteCriticalSection(&mCriticalSection);

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ClientSession::SendCompletion(DWORD transferred)
{
	if (!InitializeCriticalSectionAndSpinCount(&mCriticalSection, mSpinLockCount))
	{
		return ErrorCode::CLIENT_SESSION_LOCK_FAIL;
	}

	if (!IsConnect())
	{
		DeleteCriticalSection(&mCriticalSection);

		return ErrorCode::SUCCESS;
	}

	mSendBuffer.Pop(transferred);

	isSending = false;

	DeleteCriticalSection(&mCriticalSection);

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode NetworkLib::ClientSession::Disconnect()
{
	// TODO 최흥배
	// DisconnectEx 사용은 개인적으로 비추입니다.
	// DisconnectEx를 사용하면 OS TIME_WAIT의 동작을 생각하고 코딩해야 합니다
	// 적용 완료
	
	// mTCPSocket 객체의 멤버를 멀티스레드에서 접근해서 아래처럼 하면 문제가 있지 않을까요?
	// 적용 완료
	if (!InitializeCriticalSectionAndSpinCount(&mCriticalSection, mSpinLockCount))
	{
		return ErrorCode::CLIENT_SESSION_LOCK_FAIL;
	}

	closesocket(mTCPSocket->mSocket);
	mTCPSocket->Clear();
	mTCPSocket->Create();

	DeleteCriticalSection(&mCriticalSection);

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSession::Clear()
{
	mIndexElement = nullptr;
	mUniqueId = 0;
	isSending = false;
	mTCPSocket->Clear();
}