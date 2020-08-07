#pragma once

#include "../../NetworkLib/Config.h"


class Config : public NetworkLib::Config
{
public:
	const char* mConfigFileName = "ChatServerConfig.json";

	std::string mRedisAddress;
	uint32 mRedisPortNum = 0;
	uint32 mMaxUserNum = 0;
	uint32 mMaxRoomNum = 0;
	uint32 mMaxRoomUserNum = 0;


public:
	NetworkLib::ErrorCode Load() override;

	void staticjson_init(staticjson::ObjectHandler* handler);
};