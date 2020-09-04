#pragma once

#include "IDBManager.h"
#include "ErrorCode.h"

namespace CS
{
	class MySQLManager : IDBManager
	{
	public:
		MySQLManager() = default;
		~MySQLManager() = default;


	public:
		void Init() override;
		ErrorCode Connect() override;
		void Close() override;
	};
}
