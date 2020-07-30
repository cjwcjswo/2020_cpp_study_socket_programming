#include "ScenarioClient.h"

int main()
{
	ScenarioClient client;

	ErrorCode errorCode;
	errorCode = client.Init();
	if (ErrorCode::SUCCESS != errorCode)
	{
		GLogger->PrintConsole(Color::RED, L"Scenario Client Init Fail: %d\n", static_cast<int>(errorCode));
		return -1;
	}

	errorCode = client.Connect(L"127.0.0.1", 32452);
	if (ErrorCode::SUCCESS != errorCode)
	{
		GLogger->PrintConsole(Color::RED, L"Scenario Client Connect Fail: %d\n", static_cast<int>(errorCode));
		return -1;
	}

	errorCode = client.Disconnect();
	if (ErrorCode::SUCCESS != errorCode)
	{
		GLogger->PrintConsole(Color::RED, L"Scenario Client Disconnect Fail: %d\n", static_cast<int>(errorCode));
		return -1;
	}

	return 0;
}