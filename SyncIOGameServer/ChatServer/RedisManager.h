#pragma once

#include "Protocol.h"

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
	CS::ErrorCode Set(const char* key, const char* value);
	const char* Get(const char* key);
};


inline RedisManager* GRedisManager = new RedisManager();