#pragma once

#include <vector>
#include <queue>
#include <functional>

#include "PrimitiveTypes.h"


class ClientSession;


class ClientSessionManager
{
private:
	std::vector<ClientSession> mClientVector;
	std::queue<int32> mClientIndexPool;

	uint32 mMaxSessionSize = 0;
	uint32 mMaxSessionBufferSize = 0;


private:
	inline static unsigned long mUniqueIdGenerator = 0; // is atomic?
	

public:
	ClientSessionManager() = default;
	~ClientSessionManager() = default;


public:
	void Init(const uint32 maxClientSessionNum, const  uint32 maxSessionBufferSize) noexcept;

	uint64 GenerateUniqueId() const;
	int32 AllocClientSessionIndex();

	ClientSession* FindClientSession(const int32 index);
	ClientSession* FindClientSession(const uint64 uniqueId);

	void ConnectClientSession(ClientSession& clientSession);

	void DisconnectClientSession(const int32 index);
	void DisconnectClientSession(const uint64 uniqueId);

	void Select(std::function<void(ClientSession&)> selectFunctor);

	void CloseAll(std::function<void(ClientSession&)> closeFunctor);

	void SendAll(std::function<void(ClientSession&)> sendFunctor);
};

