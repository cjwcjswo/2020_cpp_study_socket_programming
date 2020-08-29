#pragma once

#include <WinSock2.h>


namespace NetworkLib
{
	class TCPSocket;

	enum class IOKey : ULONG_PTR
	{
		NONE = 0,
		ACCEPT = 1,
		RECEIVE = 2,
		SEND = 3,
	};

	//TODO 최흥배
	// 채팅 서버 완성 이후 하면 됩니다.
	// Interlocked Singly linked list를 사용하여 OverlappedIOContext 객체 풀을 사용합니다.
	// 자주 DeleteIOContext 메모리 할당과 해제가 발생하는데 이 비용을 감소 시킵니다.
	// Interlocked Singly linked list 사용은 제가 공유한 문서를 참고하세요
	struct OverlappedIOContext : OVERLAPPED
	{
		TCPSocket* mTCPSocket = nullptr;
		IOKey mIOKey = IOKey::NONE;
	};

	struct OverlappedIOAcceptContext : OverlappedIOContext
	{
		OverlappedIOAcceptContext(TCPSocket* tcpSocket)
		{
			memset(this, 0, sizeof(*this));
			mTCPSocket = tcpSocket;
			mIOKey = IOKey::ACCEPT;
		}
	};

	struct OverlappedIOReceiveContext : OverlappedIOContext
	{
		WSABUF mWSABuf;

		OverlappedIOReceiveContext(TCPSocket* tcpSocket)
		{
			memset(this, 0, sizeof(*this));
			mTCPSocket = tcpSocket;
			mIOKey = IOKey::RECEIVE;
		}
	};

	struct OverlappedIOSendContext : OverlappedIOContext
	{
		WSABUF mWSABuf;

		OverlappedIOSendContext(TCPSocket* tcpSocket)
		{
			memset(this, 0, sizeof(*this));
			mTCPSocket = tcpSocket;
			mIOKey = IOKey::SEND;
		}
	};

	inline void DeleteIOContext(OverlappedIOContext* context)
	{
		if (context == nullptr)
		{
			return;
		}

		switch (context->mIOKey)
		{
		case IOKey::ACCEPT:
		{
			delete reinterpret_cast<OverlappedIOAcceptContext*>(context);
			break;
		}
		case IOKey::RECEIVE:
		{
			delete reinterpret_cast<OverlappedIOReceiveContext*>(context);
			break;
		}
		}
	}
}


