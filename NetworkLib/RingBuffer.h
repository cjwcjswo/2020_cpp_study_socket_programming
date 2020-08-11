#pragma once

#include "PrimitiveTypes.h"

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
	const size_t RemainBufferSize() const;
	void Rearrange();

public:
	void Clear();
	bool Push(const char* data, const size_t size);
	bool Pop(const size_t size);

	inline const size_t DataSize() const { return mDataSize; };
	inline char* FrontData() { return mFrontMark; };
};

