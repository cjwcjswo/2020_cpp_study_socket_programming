#pragma once
#pragma comment(lib,"ws2_32")

#include <vector>
#include <queue>
#include <memory>
#include <WinSock2.h>

#include "PrimitiveTypes.h"


class ClientSession;


class ClientSessionManager
{
private:
	std::vector<ClientSession> mClientVector{};
	std::queue<int32> mClientIndexPool{};

	int mMaxSessionSize = 0;


private:
	inline static unsigned long mUniqueIdGenerator = 0; // is atomic?
	

public:
	ClientSessionManager() = default;
	~ClientSessionManager() = default;


public:
	void Init(const int maxClientSessionNum) noexcept;

	inline const std::vector<ClientSession>& ClientVector() const noexcept { return mClientVector; };

	uint64 GenerateUniqueId() const;
	int32 AllocClientSessionIndex();

	void ConnectClientSession(ClientSession& clientSession);

	void DisconnectClientSession(const int32 clientIndex);
	void DisconnectClientSession(const uint64 clientUniqueId);
	void DisconnectClientSession(const SOCKET clientSocket);
};

