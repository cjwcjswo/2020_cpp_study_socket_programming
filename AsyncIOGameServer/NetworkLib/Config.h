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
		virtual ~Config() = default;


	protected:
		const char* mConfigFileName = "NetworkConfig.json";


	public:
		std::string mIPAddress;
		uint32 mPortNum = 0;

		uint32 mMaxThreadNum = 0;
		uint32 mSpinLockCount = 0;

		uint32 mIOContextPoolSize = 0;

		uint32 mSocketAddressBufferSize = 0;
		uint32 mMaxSessionNum = 0;
		uint32 mMaxSessionBufferSize = 0;
		
		uint32 mMaxPacketBodySize = 0;

		uint32 mSendPacketCheckTick = 0;


	public:
		virtual ErrorCode Load();

		virtual void staticjson_init(staticjson::ObjectHandler* handler);
	};
}