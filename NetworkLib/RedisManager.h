#pragma once

#include <queue>
#include <mutex>
#include <thread>
#include <functional>
#include <string>

#include "RedisProtocol.h"
#include "ErrorCode.h"


struct redisContext;


namespace NetworkLib
{
	namespace Redis
	{
		class Manager
		{
		private:
			redisContext* mConnection = nullptr;

			std::unique_ptr<std::thread> mThread = nullptr;
			std::mutex mMutex;;

			std::queue<CommandRequest> mRequestQueue;
			std::queue<CommandResponse> mResponseQueue;

			uint32 mSendCheckTick = 0;
			uint32 mReceiveCheckTick = 0;
			uint32 mReceiveCheckTimeOut = 0;


		public:
			explicit Manager(uint32 sendCheckTick, uint32 receiveCheckTick, uint32 receiveCheckTimeOut) 
				: mSendCheckTick(sendCheckTick), mReceiveCheckTick(receiveCheckTick), mReceiveCheckTimeOut(receiveCheckTimeOut) {};
			~Manager();


		private:
			void ExecuteCommandProcess();
			std::string CommandRequestToString(const CommandRequest& request);


		public:
			ErrorCode Connect(const char* ipAddress, const int portNum);
			void Disconnect();

			void ExecuteCommandAsync(const CommandRequest& request);
			CommandResponse GetCommandResult();

			CommandResponse ExecuteCommand(const CommandRequest& request);
		};
				
	}
}
