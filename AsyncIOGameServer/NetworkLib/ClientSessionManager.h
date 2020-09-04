#pragma once

#include <WinSock2.h>
#include <vector>
#include <queue>
#include <atomic>

#include "SList.h"
#include "ErrorCode.h"
#include "Define.h"


namespace NetworkLib
{
	class TCPSocket;
	class ClientSession;
	struct IndexElement;

	class ClientSessionManager
	{
	private:
		std::vector<ClientSession*> mClientVector;
		SList<IndexElement>* mClientIndexPool = nullptr;

		uint32 mMaxSessionBufferSize = 0;

		std::atomic<unsigned long> mUniqueIdGenerator = 0;


	public:
		uint32 mMaxSessionSize = 0;


	public:
		ClientSessionManager() = default;
		~ClientSessionManager() = default;


	private:
		const uint64 GenerateUniqueId();
		IndexElement* AllocClientSessionIndexElement();


	public:
		ErrorCode Init(const uint32 maxClientSessionNum, const uint32 maxSessionBufferSize, const uint32 socketAddressBufferSize, const uint32 spinLockCount) noexcept;

		ClientSession* FindClientSession(const int32 index);

		ErrorCode ConnectClientSession(HANDLE iocpHandle, TCPSocket* tcpSocket);

		void DisconnectClientSession(const int32 index);

		void FlushSendClientSessionAll();
	};
}