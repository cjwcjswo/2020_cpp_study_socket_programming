#pragma once

#include "../NetworkLib/NetworkCore.h"
#include "../NetworkLib/ClientSessionManager.h"
#include "../NetworkLib/ClientSession.h"

class MockNetworkCore : public NetworkCore
{
public:
	MockNetworkCore() = default;
	~MockNetworkCore() = default;

public:
	ClientSessionManager& ClientSessionManager() { return *mClientSessionManager; };
};