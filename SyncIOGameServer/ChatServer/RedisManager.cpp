#pragma comment(lib,"hiredis")

#include <numeric>
#include <sstream>
#include <hiredis.h>

#include "../NetworkLib/Logger.h"
#include "RedisManager.h"


using namespace Redis;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Manager::~Manager()
{
	Disconnect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CS::ErrorCode Manager::Connect(const char* ipAddress, const int portNum)
{
	if (mConnection != nullptr)
	{
		return CS::ErrorCode::REDIS_ALREADY_CONNECT_STATE;
	}

	mConnection = redisConnect(ipAddress, portNum);
	if (mConnection == nullptr)
	{
		return CS::ErrorCode::REDIS_CONNECT_FAIL;
	}
	if (mConnection->err)
	{
		Disconnect();
		return CS::ErrorCode::REDIS_CONNECT_FAIL;
	}

	mThread = std::make_unique<std::thread>([&] {ExecuteCommandProcess(); });

	return CS::ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Manager::Disconnect()
{
	mThread->join();
	if (mConnection != nullptr)
	{
		redisFree(mConnection);
	}
	mConnection = nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Manager::ExecuteCommandAsync(const CommandRequest& request)
{
	std::lock_guard<std::mutex> lock(mMutex);
	mRequestQueue.push(request);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CommandResponse Manager::GetCommandResult()
{
	CommandResponse response;
	uint32 waitTick = 0;
	while (waitTick < mReceiveCheckTimeOut)
	{
		std::unique_lock<std::mutex> lock(mMutex);
		if (mResponseQueue.empty())
		{
			lock.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(mSendCheckTick));
			waitTick += mSendCheckTick;
			continue;
		}

		response = mResponseQueue.front();
		mResponseQueue.pop();
		return response;
	}

	response.mErrorCode = CS::ErrorCode::REDIS_RECEIVE_TIME_OUT;
	return response;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CommandResponse Manager::ExecuteCommand(const CommandRequest& request)
{
	CommandResponse result;
	result.mErrorCode = CS::ErrorCode::SUCCESS;

	std::string commandString = CommandRequestToString(request);
	if (commandString == "")
	{
		result.mErrorCode = CS::ErrorCode::REDIS_COMMAND_FAIL;
		return result;
	}

	redisReply* reply = (redisReply*)redisCommand(mConnection, commandString.c_str());
	if (reply == nullptr)
	{
		result.mErrorCode = CS::ErrorCode::REDIS_GET_FAIL;
		return result;
	}

	if (reply->str == nullptr)
	{
		result.mResult = "";
		mResponseQueue.push(result);
		freeReplyObject(reply);
		return result;
	}
	result.mResult = reply->str;
	if (reply->type == REDIS_REPLY_ERROR)
	{
		result.mErrorCode = CS::ErrorCode::REDIS_GET_FAIL;
		freeReplyObject(reply);
		return result;
	}

	freeReplyObject(reply);

	return result;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Manager::ExecuteCommandProcess()
{
	while (mConnection != nullptr)
	{
		std::unique_lock<std::mutex> lock(mMutex);
		if (mRequestQueue.empty())
		{
			lock.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(mSendCheckTick));
			continue;
		}

		CommandRequest request = mRequestQueue.front();
		mRequestQueue.pop();

		CommandResponse result;
		result.mErrorCode = CS::ErrorCode::SUCCESS;

		std::string commandString = CommandRequestToString(request);
		if (commandString == "")
		{
			result.mErrorCode = CS::ErrorCode::REDIS_COMMAND_FAIL;
			mResponseQueue.push(result);
			continue;
		}

		redisReply* reply = (redisReply*)redisCommand(mConnection, commandString.c_str());
		if (reply == nullptr)
		{
			result.mErrorCode = CS::ErrorCode::REDIS_GET_FAIL;
			mResponseQueue.push(result);
			freeReplyObject(reply);
			continue;
		}
		if (reply->str == nullptr)
		{
			result.mResult = "";
			mResponseQueue.push(result);
			freeReplyObject(reply);
			continue;
		}
		result.mResult = reply->str;
		if (reply->type == REDIS_REPLY_ERROR)
		{
			result.mErrorCode = CS::ErrorCode::REDIS_GET_FAIL;
			mResponseQueue.push(result);
			freeReplyObject(reply);
			continue;
		}

		mResponseQueue.push(result);
		freeReplyObject(reply);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
std::string Manager::CommandRequestToString(const CommandRequest& request)
{
	switch (request.mCommandType)
	{
	case CommandType::SET:
	{
		if (sizeof(Set) > request.mCommandBodySize)
		{
			return "";
		}

		std::ostringstream outputStream;
		outputStream << "SET ";

		Set* set = reinterpret_cast<Set*>(request.mCommandBody);
		outputStream << set->mKey << " " << set->mValue;

		if (set->mExpireTime != 0)
		{
			outputStream << " EX " << set->mExpireTime;
		}

		return outputStream.str();
	}
	case CommandType::GET:
	{
		if (request.mCommandBodySize < sizeof(Get))
		{
			return "";
		}

		std::ostringstream outputStream;
		outputStream << "GET ";

		Get* get = reinterpret_cast<Get*>(request.mCommandBody);
		outputStream << get->mKey;

		return outputStream.str();
	}
	}

	return "";
}
