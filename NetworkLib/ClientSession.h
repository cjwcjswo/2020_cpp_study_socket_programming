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
	int mSendSize = 0;

	char* mReceiveBuffer;
	RingBuffer mMessageBuffer;
	char* mSendBuffer; // TODO 최진우: SendBuffer도 링버퍼로 교체

	int32 mIndex = INVALID_INDEX;
	uint64 mUniqueId = INVALID_UNIQUE_ID;
	TCPSocket mSocket;


public:
	inline bool IsConnect() const { return mSocket.Socket() != INVALID_SOCKET; };

	void Clear();
};

