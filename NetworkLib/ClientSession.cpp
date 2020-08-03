#pragma comment(lib,"ws2_32")

#include <WinSock2.h>
#include "ClientSession.h"


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSession::ClientSession(const int32 index, const uint64 uniqueId, const SOCKET socket) : mIndex(index), mUniqueId(uniqueId), mSocket(socket)
{
	mSendBuffer = new char[BUFFER_SIZE * 2];
	mReceiveBuffer = new char[BUFFER_SIZE * 2];
	ZeroMemory(mSendBuffer, BUFFER_SIZE * 2);
	ZeroMemory(mReceiveBuffer, BUFFER_SIZE * 2);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSession::~ClientSession()
{
	delete[] mSendBuffer;
	delete[] mReceiveBuffer;
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSession::Clear()
{
	mRemainDataSize = 0;
	mPreviousReceiveBufferPos = 0;
	mSendSize = 0;

	mIndex = -1;
	mUniqueId = 0;
	mSocket.Clear();
}