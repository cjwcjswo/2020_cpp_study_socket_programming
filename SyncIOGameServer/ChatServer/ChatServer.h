#pragma once

#include "../../NetworkLib/NetworkCore.h"
#include "ErrorCode.h"


class ChatServer
{
private:
	NetworkCore mNetworkCore{};

	bool mIsRunning = false;

public:
	ChatServer() = default;
	~ChatServer() = default;


public:
	CS::ErrorCode Init();
	CS::ErrorCode Run();
	void Stop();

};