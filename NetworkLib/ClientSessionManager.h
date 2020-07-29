#pragma once
#pragma comment(lib,"ws2_32")

#include <deque>
#include <memory>
#include <WinSock2.h>

#include "PrimitiveTypes.h"


class ClientSession;


class ClientSessionManager
{
private:
	using SharedPtrClientSession = std::shared_ptr<ClientSession>;


private:
	std::deque<SharedPtrClientSession> mClientDeque{};
	int mMaxSessionSize = 0;


private:
	inline static unsigned long mUniqueIdGenerator = 0; // is atomic?
	

public:
	explicit ClientSessionManager(const int maxClientSessionSize);
	~ClientSessionManager();


private:
	unsigned long GenerateUniqueId() const;


public:
	inline const std::deque<SharedPtrClientSession>& ClientDeque() const noexcept { return this->mClientDeque; };

	ClientSession CreateClientSession(const SOCKET& clientSocket) const;

	void ConnectClientSession(const ClientSession& clientSession);

	void DisconnectClientSession(const int32 clientIndex);
	void DisconnectClientSession(const uint64 clientUniqueId);
	void DisconnectClientSession(const SOCKET clientSocket);
};

