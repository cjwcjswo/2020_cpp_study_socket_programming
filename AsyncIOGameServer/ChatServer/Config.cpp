#include <staticjson/staticjson.hpp>

#include "Config.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Config::staticjson_init(staticjson::ObjectHandler* handler)
{
	handler->add_property("RedisIPAddress", &mRedisAddress);
	handler->add_property("RedisPortNum", &mRedisPortNum);
	handler->add_property("RedisCheckSendTick", &mRedisCheckSendTick);
	handler->add_property("RedisCheckReceiveTick", &mRedisCheckReceiveTick);
	handler->add_property("RedisCheckReceiveTimeOut", &mRedisCheckReceiveTimeOut);
	handler->add_property("MaxUserNum", &mMaxUserNum);
	handler->add_property("MaxRoomNum", &mMaxRoomNum);
	handler->add_property("MaxRoomUserNum", &mMaxRoomUserNum);
	handler->set_flags(staticjson::Flags::DisallowUnknownKey);
}