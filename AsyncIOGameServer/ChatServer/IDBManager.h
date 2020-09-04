#pragma once

#include "ErrorCode.h"

namespace CS
{
	class IDBManager
	{
	public:
		IDBManager() {};
		virtual ~IDBManager() {};

		virtual void Init() = 0;
		virtual ErrorCode Connect() = 0;
		virtual void Close() = 0;
	};
}