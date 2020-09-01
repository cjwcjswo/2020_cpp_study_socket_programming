#pragma once

#include <Windows.h>


namespace NetworkLib
{
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

		bool Push(SLIST_ENTRY* entry);

		SLIST_ENTRY* Pop();
		SLIST_ENTRY* PopAll();

		unsigned short Count() const;

	};
}

