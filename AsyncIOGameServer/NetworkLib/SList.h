#pragma once

#include <Windows.h>


namespace NetworkLib
{
	template <typename T>
	class SList
	{
	private:
		SLIST_HEADER* mHeader = nullptr;
		unsigned short	mMax = 0;

	public:
		SList() = default;
		~SList();


	public:
		bool Create(const unsigned short max);

		void Destroy();

		bool Push(T* entry);

		T* Pop();
		T* PopAll();

		unsigned short Count() const;
	};

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    SList<T>::~SList()
    {
        Destroy();
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    bool SList<T>::Create(const unsigned short max)
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
    template <typename T>
    void SList<T>::Destroy()
    {
        if (mHeader != nullptr)
        {
            _aligned_free(mHeader);
            mHeader = nullptr;
        }

        mMax = 0;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    bool SList<T>::Push(T* entry)
    {
        if (mHeader != nullptr && entry != nullptr)
        {
            if (mMax > Count())
            {
                InterlockedPushEntrySList(mHeader, reinterpret_cast<SLIST_ENTRY*>(entry));
                return true;
            }
        }

        return false;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    T* SList<T>::Pop()
    {
        T* entry = nullptr;
        if (mHeader != nullptr)
        {
            entry = reinterpret_cast<T*>(InterlockedPopEntrySList(mHeader));
        }

        return entry;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    template <typename T>
    T* SList<T>::PopAll()
    {
        T* entry = nullptr;
        if (mHeader != nullptr)
        {
            entry = InterlockedFlushSList(mHeader);
        }

        return entry;
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    // TODO 최흥배
    // QueryDepthSList 이 함수를 호출할 때 상수 타임으로 크기를 알아내는지 아니면 O(n)의 시간이 걸리는지 확인 바랍니다.
    // 일단 체크
    template <typename T>
    unsigned short SList<T>::Count() const
    {
        unsigned short count = 0;
        if (mHeader != nullptr)
        {
            count = QueryDepthSList(mHeader);
        }

        return count;
    }
}

