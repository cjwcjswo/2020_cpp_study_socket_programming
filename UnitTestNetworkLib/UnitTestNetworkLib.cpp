#include "pch.h"
#include "CppUnitTest.h"
#include "MockNetworkCore.h"


using namespace Microsoft::VisualStudio::CppUnitTestFramework;


namespace UnitTestNetworkLib
{
	TEST_CLASS(UnitTestNetworkLib)
	{
	public:
		TEST_METHOD(Init)
		{
			// given
			const int maxSessionSize = 100;
			MockNetworkCore core;


			// when
			core.Init(maxSessionSize);
		

			// then
			Assert::AreEqual(maxSessionSize, static_cast<int>(core.ClientSessionManager().ClientVector().size()));
		}

		TEST_METHOD(ConnectClientSession)
		{
			// given
			MockNetworkCore core;
			core.Init(100);
			auto& manager = core.ClientSessionManager();
			ClientSession session{ 0, 921103, 10 };


			// when
			manager.ConnectClientSession(session);


			// then
			Assert::IsTrue(manager.ClientVector()[0].IsConnect());
			Assert::AreEqual(0, static_cast<int>(manager.ClientVector()[0].mIndex));
			Assert::AreEqual(10, static_cast<int>(manager.ClientVector()[0].mSocket));
			Assert::AreEqual(921103, static_cast<int>(manager.ClientVector()[0].mUniqueId));		
		}
	};
}
