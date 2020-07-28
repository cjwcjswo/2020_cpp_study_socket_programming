#include "ClientSessionManager.h"

unsigned long ClientSessionManager::GenerateUniqueId() const
{
	return ++this->unique_id_generator;
}

void ClientSessionManager::ConnectClientSession(const SOCKET& client_socket)
{
	int index = this->client_deque_.size();
	unsigned long unique_id = this->GenerateUniqueId();
	SharedPtrClientSession client_session = std::make_shared<ClientSession>(index, unique_id, client_socket);
	this->client_deque_.push_back(client_session);
	std::cout << "Connect Client Session: " << client_session->String() << std::endl;
}

void ClientSessionManager::DisconnectClientSession(const int client_index)
{
	this->client_deque_.erase(this->client_deque_.begin() + client_index);
}

void ClientSessionManager::DisconnectClientSession(const unsigned long client_unique_id)
{
	for (auto iter = client_deque_.begin(); iter != client_deque_.end(); ++iter)
	{
		if ((*iter)->unique_id() == client_unique_id)
		{
			this->client_deque_.erase(iter);
			return;
		}
	}
}

void ClientSessionManager::DisconnectClientSession(const SOCKET client_socket)
{
	for (auto iter = client_deque_.begin(); iter != client_deque_.end(); ++iter)
	{
		if ((*iter)->socket() == client_socket)
		{
			this->client_deque_.erase(iter);
			return;
		}
	}
}