#pragma once
#pragma comment(lib,"ws2_32")

#include <iostream>
#include <sstream>
#include <string>
#include <WinSock2.h>

#include "PrimitiveTypes.h"


class ClientSession
{
public:
	ClientSession();
	explicit ClientSession(const int32 index, const uint64 uniqueId, const SOCKET socket);

	~ClientSession() = default;


public:
	constexpr static int BUFFER_SIZE = 1024;
	constexpr static int INVALID_INDEX = -1;

public:
	int mRemainDataSize = 0;
	int mPreviousReceiveBufferPos = 0;
	int mSendSize = 0;

	char mReceiveBuffer[BUFFER_SIZE] = { 0, };
	char mSendBuffer[BUFFER_SIZE] = { 0, };

	int32 mIndex = INVALID_INDEX;
	uint64 mUniqueId = 0;
	SOCKET mSocket = INVALID_SOCKET;


public:
	inline bool IsConnect() const { return mSocket != INVALID_SOCKET; };

	void Clear();
};

