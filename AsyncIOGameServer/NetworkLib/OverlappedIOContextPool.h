#pragma once

#include <Windows.h>
#include "PrimitiveTypes.h"


namespace NetworkLib
{
	struct OverlappedIOContext;

	class OverlappedIOContextPool
	{
	private:
		uint32 mPoolSize = 0;
		uint32 mContextCount = 0;
		SLIST_HEADER* mContextPool = nullptr;


	public:
		OverlappedIOContextPool() = default;
		~OverlappedIOContextPool();

		bool Init(uint32 poolSize);

		void PushContext(OverlappedIOContext* context);
		OverlappedIOContext* PopContext();
	};
}


