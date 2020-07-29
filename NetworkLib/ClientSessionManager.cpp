#include <iostream>
#include "ClientSessionManager.h"
#include "ClientSession.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSessionManager::ClientSessionManager(const int maxClientSessionSize): mMaxSessionSize(maxClientSessionSize)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSessionManager::~ClientSessionManager()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long ClientSessionManager::GenerateUniqueId() const
{
	return ++mUniqueIdGenerator;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSessionManager::ConnectClientSession(const SOCKET& clientSocket)
{
	int32 index = mClientDeque.size();
	uint64 uniqueId = GenerateUniqueId();
	SharedPtrClientSession clientSession = std::make_shared<ClientSession>(index, uniqueId, clientSocket);
	mClientDeque.push_back(clientSession);
	std::cout << "Connect Client Session: " << clientSession->String() << std::endl;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSessionManager::DisconnectClientSession(const int32 clientIndex)
{
	mClientDeque.erase(mClientDeque.begin() + clientIndex);
}

void ClientSessionManager::DisconnectClientSession(const uint64 clientUniqueId)
{
	for (auto iter = mClientDeque.begin(); iter != mClientDeque.end(); ++iter)
	{
		if ((*iter)->mUniqueId == clientUniqueId)
		{
			mClientDeque.erase(iter);
			return;
		}
	}
}

void ClientSessionManager::DisconnectClientSession(const SOCKET clientSocket)
{
	for (auto iter = mClientDeque.begin(); iter != mClientDeque.end(); ++iter)
	{
		if ((*iter)->mSocket == clientSocket)
		{
			mClientDeque.erase(iter);
			return;
		}
	}
}