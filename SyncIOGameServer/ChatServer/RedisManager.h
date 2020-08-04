#pragma once

#include <queue>
#include <mutex>
#include <thread>
#include <functional>

#include "ErrorCode.h"


struct redisContext;


namespace Redis
{
	struct CommandResult
	{
		const char* mResult = "";
		CS::ErrorCode mErrorCode = CS::ErrorCode::SUCCESS;
	};

	struct CommandRequest 
	{
		const char* mCommand = nullptr; // TODO 최진우: 더 필요한 타입이 있을까? -> 추후, command랑 arguments를 분리시키는 작업
		std::function<void(const CommandResult&)> mCallBackFunc = nullptr;
	};

	
	class RedisManager
	{
	private:
		redisContext* mConnection = nullptr;

		std::unique_ptr<std::thread> mThread = nullptr;
		std::mutex mMutex;;

		std::queue<CommandRequest> mRequestQueue;

		


	public:
		RedisManager() = default;
		~RedisManager();


	private:
		void ExecuteCommandProcess();


	public:
		CS::ErrorCode Connect();
		void Disconnect();

		void ExecuteCommand(const CommandRequest& commandRequest);
		CommandResult ExecuteCommandSync(const CommandRequest& commandRequest);
	};


	inline RedisManager* GRedisManager = new RedisManager();
}
