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

		//TODO: 최흥배 이 클래스를 사용하는 곳은 많지 않습니다. std::function 등을 사용해서 글로벌 변수로 생성하지 않도록 하는 것을 추천합니다.
		// 적용 완료
	}
}
