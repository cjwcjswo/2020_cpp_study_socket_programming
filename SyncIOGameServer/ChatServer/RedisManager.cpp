//TODO: 비동기 처리 방식으로 변경한다
#pragma comment(lib,"hiredis")

#include <numeric>

#include "../../ThirdParty/hiredis/hiredis.h"
#include "../../NetworkLib/Logger.h"
#include "RedisManager.h"


using namespace Redis;

using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RedisManager::~RedisManager()
{
	Disconnect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode RedisManager::Connect(const char* ipAddress, const int portNum)
{
	if (nullptr != mConnection)
	{
		return ErrorCode::REDIS_ALREADY_CONNECT_STATE;
	}

	//TODO 최흥배: 하드코딩을 하지말고 설정 값을 받을 수 있도록 합니다~
	// 작업 완료
	mConnection = redisConnect(ipAddress, portNum);
	if (nullptr == mConnection)
	{
		return ErrorCode::REDIS_CONNECT_FAIL;
	}
	if (mConnection->err)
	{
		Disconnect();
		return ErrorCode::REDIS_CONNECT_FAIL;
	}

	mThread = std::make_unique<std::thread>([&] {ExecuteCommandProcess(); });

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RedisManager::Disconnect()
{
	mThread->join();
	if (nullptr != mConnection)
	{
		redisFree(mConnection);
	}
	mConnection = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RedisManager::ExecuteCommand(const CommandRequest& commandRequest)
{
	std::lock_guard<std::mutex> lock(mMutex);
	mRequestQueue.push(commandRequest);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CommandResult RedisManager::ExecuteCommandSync(const CommandRequest& commandRequest)
{
	redisReply* reply = (redisReply*)redisCommand(mConnection, commandRequest.mCommand);
	CommandResult result;

	if (nullptr == reply)
	{
		result.mErrorCode = ErrorCode::REDIS_GET_FAIL;
		return result;
	}

	result.mResult = reply->str;
	if (REDIS_REPLY_ERROR == reply->type)
	{
		result.mErrorCode = ErrorCode::REDIS_GET_FAIL;
		freeReplyObject(reply);
		return result;
	}

	freeReplyObject(reply);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RedisManager::ExecuteCommandProcess()
{
	while (nullptr != mConnection)
	{
		//TODO 최흥배. 스레드세이프 하지 않습니다.
		//TODO 최흥배. 큐에 데이터가 없다면 잠시 쉬어야 합니다. 지금처럼하면 불필요하게 CPU를 공회전 시켜서 CPU를 과도하게 사용합니다.
		if (mRequestQueue.empty())
		{
			continue;
		}
		CommandRequest commandRequest = mRequestQueue.front();
		mRequestQueue.pop();

		redisReply* reply = (redisReply*)redisCommand(mConnection, commandRequest.mCommand);

		CommandResult result;
		if (nullptr == reply)
		{
			result.mErrorCode = ErrorCode::REDIS_GET_FAIL;
		}
		else
		{
			result.mResult = reply->str;
			if (REDIS_REPLY_ERROR == reply->type)
			{
				result.mErrorCode = ErrorCode::REDIS_GET_FAIL;
			}
		}

		if (nullptr == commandRequest.mCallBackFunc)
		{
			freeReplyObject(reply);
			return;
		}

		commandRequest.mCallBackFunc(result);
		freeReplyObject(reply);
	}
}