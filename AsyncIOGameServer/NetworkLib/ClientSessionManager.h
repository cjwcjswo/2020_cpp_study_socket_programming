#pragma once

#include <WinSock2.h>
#include <vector>
#include <queue>

#include "PrimitiveTypes.h"
#include "ErrorCode.h"

namespace NetworkLib
{
	class ClientSession;

	class ClientSessionManager
	{
	private:
		std::vector<ClientSession> mClientVector;
		std::queue<int32> mClientIndexPool;

		uint32 mMaxSessionBufferSize = 0;

	public:
		uint32 mMaxSessionSize = 0;


	private:
		inline static unsigned long mUniqueIdGenerator = 0; // is atomic?


	public:
		ClientSessionManager() = default;
		~ClientSessionManager() = default;


	public:
		ErrorCode Init(const uint32 maxClientSessionNum, const uint32 maxSessionBufferSize) noexcept;

		uint64 GenerateUniqueId() const;
		int32 AllocClientSessionIndex();

		ClientSession* FindClientSession(const int32 index);

		ClientSession& ConnectClientSession(ClientSession& clientSession);

		void DisconnectClientSession(const int32 index);
		void DisconnectClientSession(const uint64 uniqueId);

		void FlushSendClientSessionAll();
	};
}