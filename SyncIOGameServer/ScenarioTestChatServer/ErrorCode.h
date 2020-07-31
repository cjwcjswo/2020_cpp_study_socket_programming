#pragma once

#include "../../NetworkLib/PrimitiveTypes.h"

enum class ErrorCode : uint16
{
	SUCCESS,

	WSA_START_UP_FAIL,

	SOCKET_INIT_FAIL,
	SOCKET_CONNECT_FAIL,
	SOCKET_DISCONNECT_FAIL,
	SOCKET_SEND_FAIL,

	CLIENT_SEND_BUFFER_IS_FULL,
};
