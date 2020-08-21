#pragma once

#include "TCPSocket.h"
#include "PrimitiveTypes.h"

#include "Define.h"

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
		int32 mIndex = INVALID_INDEX;
		uint64 mUniqueId = INVALID_UNIQUE_ID;
		TCPSocket* mTCPSocket = nullptr;


	public:
		bool IsConnect() const;

		void Clear();
	};


}

