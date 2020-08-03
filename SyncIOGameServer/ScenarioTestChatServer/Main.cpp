#include <wchar.h>
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

	const wchar* message = L"hi lol";
	CS::ChatRequest request;
	request.mMessageLen = static_cast<uint16>(wcslen(message));
	wmemcpy_s(request.mMessage, request.mMessageLen, message, request.mMessageLen);
	for (int i = 0; i < 50; i++)
	{
		errorCode = client.Send(static_cast<uint16>(CS::PacketId::CHAT_REQUEST), reinterpret_cast<char*>(&request), sizeof(request) - (CS::MAX_CHAT_SIZE - request.mMessageLen));
		if (ErrorCode::SUCCESS != errorCode)
		{
			GLogger->PrintConsole(Color::RED, L"Scenario Client Send Fail [Try Count: %d]: %d\n", i+1, static_cast<int>(errorCode));
			return -1;
		}
	}

	errorCode = client.Disconnect();
	if (ErrorCode::SUCCESS != errorCode)
	{
		GLogger->PrintConsole(Color::RED, L"Scenario Client Disconnect Fail: %d\n", static_cast<int>(errorCode));
		return -1;
	}

	return 0;
}