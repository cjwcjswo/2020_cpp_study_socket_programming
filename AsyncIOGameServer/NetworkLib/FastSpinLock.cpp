#define _WINSOCKAPI_
#pragma comment(lib,"Winmm")

#include "FastSpinlock.h"

#include <Windows.h>


using namespace NetworkLib;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
FastSpinLock::FastSpinLock() : mLockFlag(0)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FastSpinLock::EnterLock()
{
	// TODO 최흥배
	// 스핀락을 만든 것 같은데 직접 만든는 것보다 아래 함수가 좋습니다.
	// 이 함수는 지정 시간까지는 스핀락으로 동작하고 이후는 크리티컬섹션으로 동작합니다.
	// https://docs.microsoft.com/en-us/windows/win32/api/synchapi/nf-synchapi-initializecriticalsectionandspincount

	//Windows에 동기확 객체(lock)이 어떤 것이 있는지 알아보시기 추천합니다
	for (int nloops = 0; ; nloops++)
	{
		if (InterlockedExchange(&mLockFlag, 1) == 0) {
			return;
		}

		UINT uTimerRes = 1;
		timeBeginPeriod(uTimerRes);
		Sleep((DWORD)min(10, nloops));
		timeEndPeriod(uTimerRes);
	}

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void FastSpinLock::LeaveLock()
{
	InterlockedExchange(&mLockFlag, 0);
}