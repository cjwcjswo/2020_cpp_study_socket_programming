#pragma once

#include <string>

#include "ErrorCode.h"
#include "PrimitiveTypes.h"

namespace staticjson 
{
	class ObjectHandler;
}

namespace NetworkLib
{
	class Config
	{
	public:
		Config() = default;
		~Config() = default;


	public:
		const char* mConfigFileName = "NetworkConfig.json";

		std::string mIPAddress;
		uint32 mPortNum = 0;

		uint32 mMaxSessionNum = 0;
		uint32 mMaxSessionBufferSize = 0;

		uint32 mMaxPacketBodySize = 0;


	public:
		virtual ErrorCode Load();

		virtual void staticjson_init(staticjson::ObjectHandler* handler);
	};
}

