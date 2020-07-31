#pragma once

#include "../../NetworkLib/PrimitiveTypes.h"

namespace CS
{
	enum class ErrorCode : uint16
	{
		SUCCESS = 0,

		CHAT_SERVER_INIT_FAIL,
		CHAT_SERVER_RUN_FAIL,

		USER_MANAGER_POOL_IS_FULL,

		CHAT_SERVER_API_NOT_EXIST,
		CHAT_SERVER_INVALID_API,
	};
}