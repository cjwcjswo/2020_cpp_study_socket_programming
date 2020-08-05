#include <iostream>

#include "../../NetworkLib/Logger.h"
#include "ChatServer.h"


int main()
{
	ChatServer server;
	CS::ErrorCode errorCode = server.Init();
	if (CS::ErrorCode::SUCCESS != errorCode)
	{
		GLogger->PrintConsole(Color::LRED, "ChatServer Init Error: %d\n", static_cast<int>(errorCode));
		return -1;
	}
	errorCode = server.Run();
	if (CS::ErrorCode::SUCCESS != errorCode)
	{
		GLogger->PrintConsole(Color::LRED, "ChatServer Run Error: %d\n", static_cast<int>(errorCode));
		return -1;
	}

	return 0;
}