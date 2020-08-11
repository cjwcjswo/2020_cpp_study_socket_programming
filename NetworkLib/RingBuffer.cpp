#include <Windows.h>

#include "RingBuffer.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RingBuffer::RingBuffer(const uint32 bufferSize) : mBufferSize(bufferSize), mMaxBufferSize(bufferSize * 10 + bufferSize)
{
	Clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RingBuffer::~RingBuffer()
{
	if (nullptr != mBuffer)
	{
		delete[] mBuffer;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const size_t RingBuffer::MaxBufferSize() const
{
	return static_cast<size_t>(mMaxBufferSize - mBufferSize);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const size_t RingBuffer::RemainBufferSize() const
{
	return static_cast<size_t>(mMaxBufferSize - mDataSize);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RingBuffer::Rearrange()
{
	if (mDataSize > 0)
	{
		memmove_s(mBuffer, mDataSize, mBuffer + (mFrontMark - mBuffer), mDataSize);
	}
	mFrontMark = mBuffer;
	mRearMark = mBuffer + mDataSize;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void RingBuffer::Clear()
{
	mBuffer = new char[mMaxBufferSize];
	ZeroMemory(mBuffer, mMaxBufferSize);

	mFrontMark = mBuffer;
	mRearMark = mBuffer;
	mEndMark = mFrontMark + (mMaxBufferSize - mBufferSize);
	mDataSize = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RingBuffer::Push(const char* data, const size_t size)
{
	if (0 >= size)
	{
		return true;
	}

	if (size > RemainBufferSize())
	{
		return false;
	}

	if (mRearMark + size > mEndMark)
	{
		Rearrange();
	}

	memcpy_s(mRearMark, size, data, size);
	mRearMark += static_cast<uint32>(size);
	mDataSize += static_cast<uint32>(size);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RingBuffer::Pop(const size_t size)
{
	if (0 >= size)
	{
		return true;
	}
	
	if (size > mDataSize)
	{
		return false;
	}

	if (mFrontMark + size > mEndMark)
	{
		Rearrange();
	}

	mFrontMark += static_cast<uint32>(size);
	mDataSize -= static_cast<uint32>(size);
	return true;
}