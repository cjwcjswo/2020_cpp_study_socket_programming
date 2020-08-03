#pragma once

#include "ErrorCode.h"


struct RedisResult
{
	const char* mResult = nullptr;
	CS::ErrorCode mErrorCode = CS::ErrorCode::SUCCESS;
};

struct redisContext;

class RedisManager
{
private:
	redisContext* mConnection;


public:
	RedisManager() = default;
	~RedisManager();


public:
	CS::ErrorCode Connect();
	void Disconnect();
	RedisResult Set(const char* key, const char* value);
	RedisResult Get(const char* key);
};


inline RedisManager* GRedisManager = new RedisManager();