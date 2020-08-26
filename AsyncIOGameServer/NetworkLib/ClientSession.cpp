#pragma comment(lib,"ws2_32")

#include <WinSock2.h>
#include "ClientSession.h"


using namespace NetworkLib;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSession::ClientSession(const int32 index, const uint64 uniqueId, TCPSocket* tcpSocket) : 
	mIndex(index), mUniqueId(uniqueId), mTCPSocket(tcpSocket), mSendBufefr(1024), mReceiveBuffer(1024)
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ClientSession::~ClientSession()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
bool ClientSession::IsConnect() const
{
	if (mTCPSocket == nullptr)
	{
		return false;
	}

	if (mTCPSocket->mSocket == INVALID_SOCKET)
	{
		return false;
	}

	return true;
}

ErrorCode ClientSession::ReceiveAsync()
{
	return ErrorCode();
}

void ClientSession::ReceiveCompletion()
{
}

ErrorCode ClientSession::SendAsync()
{
	return ErrorCode();
}

void NetworkLib::ClientSession::SendCompletion()
{
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void ClientSession::Clear()
{
	mIndex = -1;
	mUniqueId = 0;
	mTCPSocket->Clear();
}