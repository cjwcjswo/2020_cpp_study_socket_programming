#include "RingBuffer.h"

#include <Windows.h>


using namespace NetworkLib;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RingBuffer::RingBuffer(const uint32 bufferSize) : mBufferSize(bufferSize), mMaxBufferSize(bufferSize * 10 + bufferSize)
{
	Clear();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
RingBuffer::~RingBuffer()
{
	if (mBuffer != nullptr)
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
char* RingBuffer::GetBuffer()
{
	if (mRearMark + RemainBufferSize() > mEndMark)
	{
		if (!Rearrange())
		{
			return nullptr;
		}
	}

	return mRearMark;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RingBuffer::Rearrange()
{
	//TODO 최흥배
	// 문제가 있습니다. 만약 mFrontMark의 위치가 mDataSize 보다 작은 경우 덮어씁니다.
	// push는 많이 되었는데 pop 속도가 느린 경우 발생할 수 있습니다.
	// 적용 완료 -> 추후 단순히 false로 리턴하는 방법 외에 유연한 방법으로 개선
	if (mDataSize > 0)
	{
		if (mDataSize > static_cast<uint32>(mFrontMark - mBuffer))
		{
			return false;
		}
		memmove_s(mBuffer, mDataSize, mBuffer + (mFrontMark - mBuffer), mDataSize);
	}
	mFrontMark = mBuffer;
	mRearMark = mBuffer + mDataSize;

	return true;
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
	if (size <= 0)
	{
		return true;
	}

	if (size > RemainBufferSize())
	{
		return false;
	}

	if (mRearMark + size > mEndMark)
	{
		if (!Rearrange())
		{
			return false;
		}
	}

	memcpy_s(mRearMark, size, data, size);
	mRearMark += static_cast<uint32>(size);
	mDataSize += static_cast<uint32>(size);

	return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool RingBuffer::Pop(const size_t size)
{
	if (size <= 0)
	{
		return true;
	}

	if (size > mDataSize)
	{
		return false;
	}

	if (mFrontMark + size > mEndMark)
	{
		if (!Rearrange())
		{
			return false;
		}
	}

	mFrontMark += static_cast<uint32>(size);
	mDataSize -= static_cast<uint32>(size);
	return true;
}