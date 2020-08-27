#pragma once

#include <mutex>
#include "TCPSocket.h"
#include "PrimitiveTypes.h"
#include "Define.h"
#include "RingBuffer.h"


namespace NetworkLib
{
	class ClientSession
	{
	public:
		explicit ClientSession(const int32 index, const uint64 uniqueId, TCPSocket* tcpSocket);

		~ClientSession();


	private:
		uint32 mMaxBufferSize = 0;
		//std::mutex mSessionMutex;


	public:
		RingBuffer mSendBuffer;
		RingBuffer mReceiveBuffer;

		int mSendPendingCount = 0;
		bool mIsConnect = false;

		int32 mIndex = INVALID_INDEX;
		uint64 mUniqueId = INVALID_UNIQUE_ID;
		TCPSocket* mTCPSocket = nullptr;


	public:
		bool IsConnect() const;

		ErrorCode ReceiveAsync();
		void ReceiveCompletion(DWORD transferred);

		ErrorCode SendAsync(const char* data, size_t length);
		ErrorCode FlushSend();
		void SendCompletion(DWORD transferred);

		void Clear();
	};
}

