#pragma once

#include <Windows.h>
#include "PrimitiveTypes.h"


namespace NetworkLib
{
	struct OverlappedIOContext;

	class OverlappedIOContextPool
	{
	private:
		uint32 mBufferSize = 0;
		uint32 mContextCount = 0;
		SLIST_HEADER mContextPool{};


	public:
		OverlappedIOContextPool(const uint32 bufferSize);
		~OverlappedIOContextPool();

		void PushContext(OverlappedIOContext* context);
		OverlappedIOContext* PopContext();
	};
}


