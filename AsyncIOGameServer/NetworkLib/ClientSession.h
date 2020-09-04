#pragma once

#include "TCPSocket.h"
#include "PrimitiveTypes.h"
#include "Define.h"
#include "RingBuffer.h"


namespace NetworkLib
{
	struct IndexElement : SLIST_ENTRY
	{
		int32 mIndex = INVALID_INDEX;
	};

	class ClientSession
	{
	public:
		ClientSession(IndexElement* indexElement, const uint64 uniqueId, TCPSocket* tcpSocket, const uint32 spinLockCount, const uint32 maxBufferSize);

		~ClientSession();


	private:
		CRITICAL_SECTION mCriticalSection;
		uint32 mMaxBufferSize = 0;
		uint32 mSpinLockCount = 0;
		

	public:
		RingBuffer mSendBuffer;
		RingBuffer mReceiveBuffer;

		bool isSending = false;
		bool mIsConnect = false;

		IndexElement* mIndexElement = nullptr;
		uint64 mUniqueId = INVALID_UNIQUE_ID;
		TCPSocket* mTCPSocket = nullptr;


	public:
		bool IsConnect() const;

		ErrorCode ReceiveAsync();
		void ReceiveCompletion(DWORD transferred);

		ErrorCode SendAsync(const char* data, size_t length);
		ErrorCode FlushSend();
		ErrorCode SendCompletion(DWORD transferred);

		ErrorCode Disconnect();

		void Clear();
	};
}

