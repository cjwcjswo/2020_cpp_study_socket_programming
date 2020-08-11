#pragma once

#include <string>

#include "PrimitiveTypes.h"
#include "ErrorCode.h"


namespace NetworkLib
{
	namespace Redis 
	{
		enum class CommandType : uint16
		{
			NONE = 0,
			SET = 1,
			GET = 2,
		};

		const int KEY_MAX_SIZE = 1024;
		const int VALUE_MAX_SIZE = 4096;
		const int REQUEST_MAX_SIZE = 4096;
		const int RESULT_MAX_SIZE = 4096;

		struct CommandRequest
		{
			CommandType mCommandType = CommandType::NONE;
			char* mCommandBody = nullptr;
			int mCommandBodySize = 0;
		};

		struct CommandResponse
		{
			CommandType mCommandType = CommandType::NONE;
			ErrorCode mErrorCode = ErrorCode::SUCCESS;
			std::string mResult;
		};

		struct Set
		{
			char mKey[KEY_MAX_SIZE] = { 0, };
			char mValue[VALUE_MAX_SIZE] = { 0, };
			uint32 mExpireTime = 0;
		};

		struct Get
		{
			char mKey[KEY_MAX_SIZE] = { 0, };
		};
	}
}
