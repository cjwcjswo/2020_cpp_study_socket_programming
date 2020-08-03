#pragma once

#include "../../NetworkLib/PrimitiveTypes.h"


namespace CS
{
	enum class ErrorCode : uint16
	{
		SUCCESS = 0,

		CHAT_SERVER_INIT_FAIL,
		CHAT_SERVER_RUN_FAIL,

		REDIS_CONNECT_FAIL,
		REDIS_SET_FAIL,
		REDIS_GET_FAIL,

		USER_MANAGER_POOL_IS_FULL,

		CHAT_SERVER_API_NOT_EXIST,
		CHAT_SERVER_INVALID_API,

		INVALID_USER,
		LOGIN_AUTH_FAIL,
	};
}