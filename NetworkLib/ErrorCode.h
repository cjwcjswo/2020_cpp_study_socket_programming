#pragma once
enum class ErrorCode
{
	SUCCESS = 0,

	WSA_START_UP_FAIL,

	SOCKET_INIT_FAIL,
	SOCKET_BIND_FAIL,
	SOCKET_LISTEN_FAIL,
	SOCKET_SELECT_FAIL,
	SOCKET_SELECT_RESULT_ZERO,
	SOCKET_ACCEPT_CLIENT_FAIL,
};
