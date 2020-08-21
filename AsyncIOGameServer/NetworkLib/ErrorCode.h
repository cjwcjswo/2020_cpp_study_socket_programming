#pragma once

#include "PrimitiveTypes.h"

namespace NetworkLib
{
	enum class ErrorCode : uint16
	{
		SUCCESS = 0,

		WSA_START_UP_FAIL,

		SOCKET_INIT_FAIL,
		SOCKET_BIND_FAIL,
		SOCKET_LISTEN_FAIL,
		SOCKET_INIT_REUSE_ADDR_FAIL,
		SOCKET_ACCEPT_ASYNC_FAIL,

		CLIENT_SESSION_NOT_EXIST,
	};
}
