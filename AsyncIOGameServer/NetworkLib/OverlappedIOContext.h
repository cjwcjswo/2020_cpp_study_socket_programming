#pragma once

#include <WinSock2.h>

#include "SList.h"


namespace NetworkLib
{
	class TCPSocket;
	class ClientSession;

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
	// 적용 완료
	struct OverlappedIOContext : OVERLAPPED
	{
		IOKey mIOKey = IOKey::NONE;
		TCPSocket* mTCPSocket = nullptr;
		ClientSession* mSession = nullptr;
		WSABUF mWSABuf;

		OverlappedIOContext()
		{
			Clear();
		}

		void Clear()
		{
			memset(this, 0, sizeof(*this));
		}
	};

	 inline SList<OverlappedIOContext>* GIOContextPool = new SList<OverlappedIOContext>;
}