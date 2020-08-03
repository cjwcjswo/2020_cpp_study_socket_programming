//TODO: 비동기 처리 방식으로 변경한다
#pragma comment(lib,"hiredis")

#include "../../ThirdParty/hiredis/hiredis.h"
#include "RedisManager.h"


using namespace CS;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RedisManager::~RedisManager()
{
	Disconnect();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode RedisManager::Connect()
{
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

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RedisManager::Disconnect()
{
	redisFree(mConnection);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RedisResult RedisManager::Set(const char* key, const char* value)
{
	RedisResult result;

	redisReply* reply = (redisReply*)redisCommand(mConnection, "SET %s %s", key, value);
	if (nullptr == reply)
	{
		result.mErrorCode = ErrorCode::REDIS_SET_FAIL;
		return result;
	}
	if (REDIS_REPLY_ERROR == reply->type)
	{
		result.mErrorCode = ErrorCode::REDIS_SET_FAIL;
		freeReplyObject(reply);
		return result;
	}
	
	result.mResult = reply->str;
	freeReplyObject(reply);

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RedisResult RedisManager::Get(const char* key)
{
	RedisResult result;

	redisReply* reply = (redisReply*)redisCommand(mConnection, "GET %s", key);
	if (nullptr == reply)
	{
		result.mErrorCode = ErrorCode::REDIS_GET_FAIL;
		return result;
	}
	if (REDIS_REPLY_ERROR == reply->type)
	{
		result.mErrorCode = ErrorCode::REDIS_GET_FAIL;
		freeReplyObject(reply);
		return result;
	}

	result.mResult = reply->str;
	freeReplyObject(reply);

	return result;
}