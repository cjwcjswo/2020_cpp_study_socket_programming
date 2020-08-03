#pragma once

#include "TCPSocket.h"
#include "PrimitiveTypes.h"


class ClientSession
{
public:
	explicit ClientSession(const int32 index, const uint64 uniqueId, const SOCKET socket);

	~ClientSession();


public:
	constexpr static int BUFFER_SIZE = 1024;
	constexpr static int INVALID_INDEX = -1;


public:
	int mRemainDataSize = 0;
	int mPreviousReceiveBufferPos = 0;
	int mSendSize = 0;

	char* mReceiveBuffer = nullptr;
	char* mSendBuffer = nullptr;

	int32 mIndex = INVALID_INDEX;
	uint64 mUniqueId = 0;
	TCPSocket mSocket;


public:
	inline bool IsConnect() const { return mSocket.Socket() != INVALID_SOCKET; };

	void Clear();
};

