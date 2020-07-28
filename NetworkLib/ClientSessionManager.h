#pragma once
#pragma comment(lib,"ws2_32")
#include <deque>
#include <memory>
#include <WinSock2.h>

#include "ClientSession.h"

using SharedPtrClientSession = std::shared_ptr<ClientSession>;

class ClientSessionManager
{
private:
	unsigned long GenerateUniqueId() const;

private:
	std::deque<SharedPtrClientSession> client_deque_{};
	inline static unsigned long unique_id_generator = 0; // is atomic?
	int max_session_size_ = 0;

public:
	explicit ClientSessionManager(int max_client_session_size) : max_session_size_(max_client_session_size) {};

	const std::deque<SharedPtrClientSession>& client_deque() const { return this->client_deque_; };
	void ConnectClientSession(const SOCKET& client_socket);
	void DisconnectClientSession(const int client_index);
	void DisconnectClientSession(const unsigned long client_unique_id);
	void DisconnectClientSession(const SOCKET client_socket);
};

