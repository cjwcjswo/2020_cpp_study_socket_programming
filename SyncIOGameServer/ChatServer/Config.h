#pragma once

#include "../../NetworkLib/Config.h"


class Config : public NetworkLib::Config
{
public:
	Config() { mConfigFileName = "ChatServerConfig.json"; };
	virtual ~Config() = default;


public:
	std::string mRedisAddress;
	uint32 mRedisPortNum = 0;
	uint32 mRedisCheckSendTick = 0;
	uint32 mRedisCheckReceiveTick = 0;
	uint32 mRedisCheckReceiveTimeOut = 0;

	uint32 mMaxUserNum = 0;
	uint32 mMaxRoomNum = 0;
	uint32 mMaxRoomUserNum = 0;

public:
	void staticjson_init(staticjson::ObjectHandler* handler) override;
};