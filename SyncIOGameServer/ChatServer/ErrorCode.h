#pragma once

#include "../../NetworkLib/PrimitiveTypes.h"

namespace CS
{
	enum class ErrorCode : uint16
	{
		SUCCESS = 0,

		CHAT_SERVER_INIT_FAIL,
		CHAT_SERVER_RUN_FAIL,
	};
}