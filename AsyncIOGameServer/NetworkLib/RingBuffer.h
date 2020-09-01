#pragma once

#include "PrimitiveTypes.h"


namespace NetworkLib
{
	class RingBuffer
	{
	public:
		explicit RingBuffer(const uint32 bufferSize);
		~RingBuffer();


	private:
		char* mBuffer = nullptr;
		uint32 mBufferSize = 0;
		uint32 mMaxBufferSize = 0;
		uint32 mDataSize = 0;
		char* mFrontMark = nullptr;
		char* mRearMark = nullptr;
		char* mEndMark = nullptr;


	private:
		const size_t MaxBufferSize() const;
		bool Rearrange();

	public:
		void Clear();
		bool Push(const char* data, const size_t size);
		bool Pop(const size_t size);

		const size_t RemainBufferSize() const;

		inline const size_t DataSize() const { return mDataSize; };
		inline char* FrontData() { return mFrontMark; };
		inline void Commit(size_t size) { mRearMark += size; mDataSize += static_cast<uint32>(size); };
		char* GetBuffer();
	};
}
