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
ErrorCode RedisManager::Connect()
{
	if (nullptr != mConnection)
	{
		return ErrorCode::REDIS_ALREADY_CONNECT_STATE;
	}

	mConnection = redisConnect("127.0.0.1", 6379);
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
	redisFree(mConnection);
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