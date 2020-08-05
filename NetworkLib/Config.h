#pragma once

#include "../ThirdParty/rapidjson/document.h"
#include "ErrorCode.h"
#include "PrimitiveTypes.h"


namespace NetworkLib
{
	class Config
	{
	public:
		Config() = default;
		~Config() = default;


	private:
		inline static const char* CONFIG_FILE_NAME = "NetworkConfig.json";


	private:
		ErrorCode LoadFromMemberIter(const rapidjson::Value::ConstMemberIterator& iter);


	public:
		const wchar* mIPAddress = nullptr;
		uint16 mPortNum = 0;

		uint32 mMaxSessionNum = 0;
		uint32 mMaxSessionBufferSize = 0;

		uint16 mMaxPacketBodySize = 0;


	public:
		ErrorCode Load();

	};
}

