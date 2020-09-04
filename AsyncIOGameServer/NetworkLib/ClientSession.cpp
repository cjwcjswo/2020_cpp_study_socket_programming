#pragma comment(lib, "mswsock.lib")  

#include "ClientSession.h"

#include <WinSock2.h>
#include <mswsock.h>
#include "OverlappedIOContext.h"


using namespace NetworkLib;


//TODO 최흥배
// mIndexElement(indexElement), mUniqueId(uniqueId)가 값을 대입하는 것이 성능적으로 좋을 수는 있지만 코드 보기에는 별로 좋지 않습니다.
// ClientSession은 매번 생성하는 것이 아니니 성능 이득은 없습니다. 코드 보기를 더 좋게 하는 것이 좋습니다.
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

	//TODO 최흥배
	// 사용법이 틀렸습니다. 아래 글을 참고하세요
	// https://docs.microsoft.com/en-us/windows/win32/sync/using-critical-section-objects
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
	mReceiveBuffer.Commit(transferred);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode ClientSession::SendAsync(const char* data, size_t length)
{
	//TODO 최흥배
	// 사용법 에러
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

	//TODO 최흥배
	// 사용법 에러
	if (!InitializeCriticalSectionAndSpinCount(&mCriticalSection, mSpinLockCount))
	{
		return ErrorCode::CLIENT_SESSION_LOCK_FAIL;
	}

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

	const int MAX_SEGMENT_SIZE = 1024; // 넉넉하게 1K
	ULONG remainSize = static_cast<ULONG>(mSendBuffer.DataSize());
	int pos = 0;

	// TODO 최흥배
	// n-send, 1-send 이야기 기억나시나요? send 전용 스레드를 만들어서 패킷을 보낼 때는 1-send를 쉽게 적용할 수 있으므로 대부분 1-send를 합니다.
	// 아래는 n-send를 하고 있습니다. 1-send로 바꾸어보세요. send 요청 후 send가 완료된 이후에만 다시 WSASend를 호출합니다.
	// 위 코드를 보면 1-send를 하기 위한 조건은 거의 다 갖추고 있습니다.
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