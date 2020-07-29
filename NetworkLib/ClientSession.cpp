#include "ClientSession.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSession::ClientSession()
{
	Clear();
}

ClientSession::ClientSession(const int32 index, const uint64 uniqueId, const SOCKET socket) : mIndex(index), mUniqueId(uniqueId), mSocket(socket)
{
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSession::Clear()
{
	mRemainDataSize = 0;
	mPreviousReceiveBufferPos = 0;
	mSendSize = 0;

	mIndex = -1;
	mUniqueId = 0;
	mSocket = INVALID_SOCKET;
}