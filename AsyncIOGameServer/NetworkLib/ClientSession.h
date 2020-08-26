#pragma once

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


	public:
		RingBuffer mSendBufefr;
		RingBuffer mReceiveBuffer;

		int32 mIndex = INVALID_INDEX;
		uint64 mUniqueId = INVALID_UNIQUE_ID;
		TCPSocket* mTCPSocket = nullptr;


	public:
		bool IsConnect() const;

		ErrorCode ReceiveAsync();
		void ReceiveCompletion();

		ErrorCode SendAsync();
		void SendCompletion();

		void Clear();
	};


}

