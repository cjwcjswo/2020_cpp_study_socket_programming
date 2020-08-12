#pragma once

#include "TCPSocket.h"
#include "PrimitiveTypes.h"
#include "Define.h"
#include "RingBuffer.h"


class ClientSession
{
public:
	explicit ClientSession(const int32 index, const uint64 uniqueId, const SOCKET socket, const uint32 bufferSize);

	~ClientSession();


private:
	uint32 mMaxBufferSize = 0;


public:
	char* mReceiveBuffer;
	RingBuffer mMessageBuffer;
	RingBuffer mSendBuffer;

	int32 mIndex = INVALID_INDEX;
	uint64 mUniqueId = INVALID_UNIQUE_ID;
	TCPSocket mSocket;


public:
	inline bool IsConnect() const { return mSocket.Socket() != INVALID_SOCKET; };

	void Clear();
};

