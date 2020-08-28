﻿#pragma once

#include "PrimitiveTypes.h"

namespace NetworkLib
{
	enum class ErrorCode : uint16
	{
		SUCCESS = 0,

		WSA_START_UP_FAIL = 300,
		CREATE_EVENT_FAIL = 301,

		SOCKET_INIT_FAIL = 400,
		SOCKET_BIND_FAIL = 401,
		SOCKET_LISTEN_FAIL = 402,
		SOCKET_INIT_REUSE_ADDR_FAIL = 403,
		SOCKET_ACCEPT_ASYNC_FAIL = 404,

		CLIENT_SESSION_NOT_EXIST = 500,
		CLIENT_SESSION_NOT_CONNECTED = 501,
		CLIENT_SESSION_RECEIVE_FAIL = 502,
		CLIENT_SESSION_RECEIVE_BUFFER_FULL = 503,
		CLIENT_SESSION__RECEIVE_MAX_PACKET_SIZE = 504,
		CLIENT_SESSION_SEND_FAIL = 505,
		CLIENT_SESSION_SEND_BUFFER_FULL = 506,
	};
}
