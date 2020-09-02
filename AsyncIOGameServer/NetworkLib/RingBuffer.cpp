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

// TODO 최흥배
// Clear는 이 객체가 재사용되면서 매번 호출되는 경우에 적합합 이름 같은데 코드에서는 클래스 생성 떄 한번 호출합니다.
// Init나 Create가 맞지 않을까 생각합니다
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