#include "OverlappedIOContextPool.h"


using namespace NetworkLib;



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OverlappedIOContextPool::~OverlappedIOContextPool()
{
    if (mContextPool != nullptr)
    {
        _aligned_free(mContextPool);
        mContextPool = nullptr;
    }

    mPoolSize = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NetworkLib::OverlappedIOContextPool::Init(uint32 poolSize)
{
    mPoolSize = poolSize;

    if (mContextPool != nullptr)
    {
        return false;
    }

    mContextPool = reinterpret_cast<SLIST_HEADER*>(_aligned_malloc(sizeof(SLIST_HEADER), MEMORY_ALLOCATION_ALIGNMENT));
    if (mContextPool == nullptr)
    {
        return false;
    }

    InitializeSListHead(mContextPool);
    
    for (int i = 0; i < poolSize; ++i)
    {

    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void OverlappedIOContextPool::PushContext(OverlappedIOContext* context)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
OverlappedIOContext* OverlappedIOContextPool::PopContext()
{
    return nullptr;
}
