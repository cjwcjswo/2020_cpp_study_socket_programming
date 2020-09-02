#include "SList.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NetworkLib::SList::~SList()
{
    Destroy();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NetworkLib::SList::Create(const unsigned short max)
{
    if (mHeader != nullptr)
    {
        return false;
    }

    mHeader = reinterpret_cast<SLIST_HEADER*>(_aligned_malloc(sizeof(SLIST_HEADER), MEMORY_ALLOCATION_ALIGNMENT));
    if (mHeader == nullptr)
    {
        return false;
    }

    InitializeSListHead(mHeader);
    mMax = max;

    return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkLib::SList::Destroy()
{
    if (mHeader != nullptr)
    {
        _aligned_free(mHeader);
        mHeader = nullptr;
    }

    mMax = 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool NetworkLib::SList::Push(SLIST_ENTRY* entry)
{
    if (mHeader != nullptr && entry != nullptr)
    {
        if (mMax > Count())
        {
            InterlockedPushEntrySList(mHeader, entry);
            return true;
        }
    }

    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SLIST_ENTRY* NetworkLib::SList::Pop()
{
    SLIST_ENTRY* entry = nullptr;
    if (mHeader != nullptr)
    {
        entry = InterlockedPopEntrySList(mHeader);
    }

    return entry;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
SLIST_ENTRY* NetworkLib::SList::PopAll()
{
    SLIST_ENTRY* entry = nullptr;
    if (mHeader != nullptr)
    {
        entry = ::InterlockedFlushSList(mHeader);
    }

    return entry;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned short NetworkLib::SList::Count() const
{
    unsigned short count = 0;
    if (mHeader != nullptr)
    {
        // TODO 최흥배
        // QueryDepthSList 이 함수를 호출할 때 상수 타임으로 크기를 알아내는지 아니면 O(n)의 시간이 걸리는지 확인 바랍니다.
        count = QueryDepthSList(mHeader);
    }

    return count;
}