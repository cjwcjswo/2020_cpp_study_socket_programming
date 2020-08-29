﻿#pragma once

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

	//TODO �����
	// ä�� ���� �ϼ� ���� �ϸ� �˴ϴ�.
	// Interlocked Singly linked list�� ����Ͽ� OverlappedIOContext ��ü Ǯ�� ����մϴ�.
	// ���� DeleteIOContext �޸� �Ҵ�� ������ �߻��ϴµ� �� ����� ���� ��ŵ�ϴ�.
	// Interlocked Singly linked list ����� ���� ������ ������ �����ϼ���
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


