#pragma comment(lib,"ws2_32")

#include <WinSock2.h>
#include "ClientSession.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSession::ClientSession(const int32 index, const uint64 uniqueId, const SOCKET socket, const uint32 bufferSize) 
	: mIndex(index), mUniqueId(uniqueId), mSocket(socket), mMessageBuffer(bufferSize)
{
	mReceiveBuffer = new char[bufferSize];
	mSendBuffer = new char[bufferSize];
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSession::~ClientSession()
{
	if (nullptr != mReceiveBuffer)
	{
		delete[] mReceiveBuffer;
	}
	if (nullptr != mSendBuffer)
	{
		delete[] mSendBuffer;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSession::Clear()
{
	mSendSize = 0;

	mIndex = -1;
	mUniqueId = 0;

	mSocket.Clear();
	mMessageBuffer.Clear();
}