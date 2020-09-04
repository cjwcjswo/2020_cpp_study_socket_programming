#pragma once

#include "../NetworkLib/PrimitiveTypes.h"


namespace CS
{
	enum class ErrorCode : uint16
	{
		SUCCESS = 200,

		CHAT_SERVER_INIT_FAIL = 300,
		CHAT_SERVER_RUN_FAIL,

		REDIS_ALREADY_CONNECT_STATE = 400,
		REDIS_COMMAND_FAIL,
		REDIS_RECEIVE_TIME_OUT,
		REDIS_CONNECT_FAIL,
		REDIS_SET_FAIL,
		REDIS_GET_FAIL,

		USER_MANAGER_POOL_IS_FULL = 500,

		CHAT_SERVER_API_NOT_EXIST = 600,
		CHAT_SERVER_INVALID_API,

		USER_NOT_CONNECTED_STATE = 700,
		USER_ALREADY_CONNECTED_STATE,
		USER_NOT_LOGIN_STATE,
		USER_ALREADY_LOGIN_STATE,
		USER_NOT_ROOM_ENTER_STATE,
		USER_ALREADY_IN_THE_ROOM,
		USER_IS_INVALID,
		USER_LOGIN_AUTH_FAIL,

		ROOM_NOT_EXIST = 800,
		ROOM_IS_FULL,

	};
}