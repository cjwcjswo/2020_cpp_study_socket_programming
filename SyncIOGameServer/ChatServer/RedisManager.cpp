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
ErrorCode RedisManager::Set(const char* key, const char* value)
{
	redisReply* reply = (redisReply*)redisCommand(mConnection, "SET %s %s", key, value);
	if (nullptr == reply)
	{
		return ErrorCode::REDIS_SET_FAIL;
	}
	if (REDIS_REPLY_ERROR == reply->type)
	{
		freeReplyObject(reply);
		return ErrorCode::REDIS_SET_FAIL;
	}

	freeReplyObject(reply);

	return ErrorCode::SUCCESS;
}