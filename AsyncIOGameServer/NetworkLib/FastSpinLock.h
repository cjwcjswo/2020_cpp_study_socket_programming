#pragma once


namespace NetworkLib
{
	class FastSpinLock
	{
	private:
		volatile long mLockFlag;


	public:
		FastSpinLock();
		~FastSpinLock() = default;

		void EnterLock();
		void LeaveLock();
	};

	class FastSpinlockGuard
	{
	private:
		FastSpinLock& mLock;


	public:
		FastSpinlockGuard(FastSpinLock& lock) : mLock(lock)
		{
			mLock.EnterLock();
		}

		~FastSpinlockGuard()
		{
			mLock.LeaveLock();
		}
	};
}