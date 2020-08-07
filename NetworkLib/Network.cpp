#include "Network.h"
#include "TCPSocket.h"
#include "Define.h"
#include "ClientSessionManager.h"
#include "ClientSession.h"
#include "Config.h"


using namespace NetworkLib;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Network::~Network()
{
	Stop();

	if (nullptr != mConfig)
	{
		delete mConfig;
	}
	if (nullptr != mAcceptSocket)
	{
		delete mAcceptSocket;
	}
	if (nullptr != mClientSessionManager)
	{
		delete mClientSessionManager;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Network::AcceptClient()
{
	TCPSocket clientSocket = mAcceptSocket->Accept();
	if (INVALID_SOCKET == clientSocket.Socket())
	{
		return ErrorCode::SOCKET_ACCEPT_CLIENT_FAIL;
	}

	ClientSession clientSession(INVALID_INDEX, INVALID_UNIQUE_ID, INVALID_SOCKET, mConfig->mMaxSessionBufferSize);
	clientSession.mSocket = clientSocket;

	clientSession.mIndex = mClientSessionManager->AllocClientSessionIndex();
	if (INVALID_INDEX == clientSession.mIndex)
	{
		CloseSession(ErrorCode::SOCKET_INDEX_POOL_IS_FULL, clientSession);
		return ErrorCode::SOCKET_INDEX_POOL_IS_FULL;
	}

	clientSession.mSocket.SetLingerMode();

	if (ErrorCode::SUCCESS != mAcceptSocket->SetNonBlockingMode())
	{
		return ErrorCode::SOCKET_SET_FIONBIO_FAIL;
	}

	FD_SET(clientSession.mSocket.Socket(), &mReadSet);
	
	clientSession.mUniqueId = mClientSessionManager->GenerateUniqueId();

	mClientSessionManager->ConnectClientSession(clientSession);

	PushReceivePacket(ReceivePacket{ clientSession.mIndex, clientSession.mUniqueId, static_cast<uint16>(PacketId::CONNECT), 0, nullptr});

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Network::CheckSelectResult(int selectResult)
{
	if (0 == selectResult)
	{
		return ErrorCode::SOCKET_SELECT_RESULT_ZERO;
	}

	if (selectResult < 0)
	{
		return ErrorCode::SOCKET_SELECT_FAIL;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Network::SelectClient(const fd_set& readSet, const fd_set& writeSet)
{
	for (ClientSession& clientSession : mClientSessionManager->ClientVector())
	{
		if (!clientSession.IsConnect())
		{
			continue;
		}

		TCPSocket clientSocket = clientSession.mSocket;
		if (FD_ISSET(clientSocket.Socket(), &readSet))
		{
			ErrorCode errorCode = ReceiveClient(clientSession);
			if (ErrorCode::SUCCESS != errorCode)
			{
				CloseSession(errorCode, clientSession);
				continue;
			}
		}
		if (FD_ISSET(clientSocket.Socket(), &writeSet))
		{
			ErrorCode errorCode = SendClient(clientSession);
			if (ErrorCode::SUCCESS != errorCode)
			{
				CloseSession(errorCode, clientSession);
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Network::ReceiveClient(ClientSession& clientSession)
{
	//TODO 최흥배: mReceiveBuffer의 크기를 ClientSession::BUFFER_SIZE 2배로 잡아서 남은 데이터를 앞으로 복사하지 않고 남은 부분 다음에 받도록 합니다. 복사를 줄이기 위해서 입니다.
	uint32 receivePos = clientSession.mPreviousReceiveBufferPos + clientSession.mRemainDataSize;

	// 남은 데이터가 버퍼 사이즈보다 클 경우 앞으로 복사
	if (receivePos > mConfig->mMaxSessionBufferSize)
	{
		memcpy_s(clientSession.mReceiveBuffer, clientSession.mRemainDataSize, &clientSession.mReceiveBuffer[clientSession.mPreviousReceiveBufferPos], clientSession.mRemainDataSize);
		clientSession.mPreviousReceiveBufferPos = 0;
		receivePos = clientSession.mRemainDataSize;
	}

	int length = clientSession.mSocket.Receive(&clientSession.mReceiveBuffer[receivePos], mConfig->mMaxSessionBufferSize);
	if (0 == length)
	{
		return ErrorCode::SOCKET_RECEIVE_ZERO;
	}
	if (length < 0)
	{
		auto netErrorCode = WSAGetLastError();
		if (WSAEWOULDBLOCK == netErrorCode)
		{
			return ErrorCode::SUCCESS;
		}
		else
		{
			return ErrorCode::SOCKET_RECEIVE_FAIL;
		}
	}

	clientSession.mRemainDataSize += length;
	PacketHeader* header;

	while (clientSession.mRemainDataSize >= PACKET_HEADER_SIZE)
	{
		header = reinterpret_cast<PacketHeader*>(&clientSession.mReceiveBuffer[receivePos]);
		receivePos += PACKET_HEADER_SIZE;
		uint16 requireBodySize = header->mPacketSize - PACKET_HEADER_SIZE;

		if (requireBodySize > 0)
		{
			if (requireBodySize > clientSession.mRemainDataSize)
			{
				receivePos -= PACKET_HEADER_SIZE;
				break;
			}
			if (requireBodySize > mConfig->mMaxPacketBodySize)
			{
				return ErrorCode::SOCKET_RECEIVE_MAX_PACKET_SIZE;
			}
		}

		ReceivePacket receivePacket = { clientSession.mIndex, clientSession.mUniqueId, header->mPacketId, 0, nullptr };
		if (requireBodySize > 0)
		{
			receivePacket.mBodyDataSize = requireBodySize;
			receivePacket.mBodyData = &clientSession.mReceiveBuffer[receivePos];
		}
		PushReceivePacket(receivePacket);
		receivePos += requireBodySize;
		clientSession.mRemainDataSize -= header->mPacketSize;
	}

	clientSession.mPreviousReceiveBufferPos = receivePos;

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Network::SelectProcess()
{
	ErrorCode errorCode;
	while (mIsRunning)
	{
		fd_set readSet = mReadSet;
		fd_set writeSet = mWriteSet;

		// Block
		int selectResult = select(NULL, &readSet, &writeSet, nullptr, nullptr);

		errorCode = CheckSelectResult(selectResult);
		if (ErrorCode::SUCCESS != errorCode)
		{
			continue;
		}

		if (FD_ISSET(mAcceptSocket->Socket(), &readSet))
		{
			errorCode = AcceptClient();
			if (ErrorCode::SUCCESS != errorCode)
			{
				continue;
			}
		}

		SelectClient(readSet, writeSet);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Network::SendClient(ClientSession& clientSession)
{
	if (clientSession.mSendSize <= 0)
	{
		return ErrorCode::SUCCESS;
	}

	
	int length = clientSession.mSocket.Send(clientSession.mSendBuffer, clientSession.mSendSize);
	if (length <= 0)
	{
		return ErrorCode::SOCKET_SEND_SIZE_ZERO;
	}

	if (clientSession.mSendSize > length)
	{
		auto remain = clientSession.mSendSize - length;
		memmove_s(clientSession.mSendBuffer, remain, &clientSession.mSendBuffer[length], remain);
		clientSession.mSendSize -= length;
	}
	else
	{
		clientSession.mSendSize = 0;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Network::PushReceivePacket(const ReceivePacket receivePacket)
{
	std::lock_guard<std::mutex> lock(mPacketMutex);
	mReceivePacketQueue.push(receivePacket);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Network::CloseSession(const ErrorCode errorCode, ClientSession& clientSession)
{
	if (!clientSession.IsConnect())
	{
		return;
	}

	FD_CLR(clientSession.mSocket.Socket(), &mReadSet);

	clientSession.mSocket.Close();

	if (INVALID_INDEX == clientSession.mIndex)
	{
		return;
	}

	PushReceivePacket(ReceivePacket{ clientSession.mIndex, clientSession.mUniqueId, static_cast<uint16>(PacketId::DISCONNECT), 0, nullptr });
	mClientSessionManager->DisconnectClientSession(clientSession.mIndex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Network::Init()
{
	ErrorCode errorCode;

	mConfig = new Config();
	errorCode = mConfig->Load();
	if (ErrorCode::SUCCESS != errorCode)
	{
		return errorCode;
	}

	WSADATA wsaData;

	if (SOCKET_ERROR == WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		return ErrorCode::WSA_START_UP_FAIL;
	}

	// Client Accept Socket Init
	mAcceptSocket = new TCPSocket{INVALID_SOCKET};
	errorCode = mAcceptSocket->Create();
	if (ErrorCode::SUCCESS != errorCode)
	{
		return errorCode;
	}

	errorCode = mAcceptSocket->Bind(mConfig->mIPAddress, mConfig->mPortNum);
	if (ErrorCode::SUCCESS != errorCode)
	{
		return errorCode;
	}

	errorCode = mAcceptSocket->Listen();
	if (ErrorCode::SUCCESS != errorCode)
	{
		return errorCode;
	}

	mClientSessionManager = new ClientSessionManager();
	mClientSessionManager->Init(mConfig->mMaxSessionNum, mConfig->mMaxSessionBufferSize);

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Network::Run()
{
	FD_ZERO(&mReadSet);
	FD_ZERO(&mWriteSet);
	FD_SET(mAcceptSocket->Socket(), &mReadSet);

	// Start Thread
	mIsRunning = true;
	mSelectThread = std::make_unique<std::thread>([&]() {SelectProcess();});

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Network::Stop()
{
	// Stop Accept Socket
	if (nullptr != mAcceptSocket)
	{
		mAcceptSocket->Close();
	}

	if (mIsRunning)
	{
		for (ClientSession clientSession : mClientSessionManager->ClientVector())
		{
			if (clientSession.IsConnect())
			{
				CloseSession(ErrorCode::SUCCESS, clientSession);
			}
		}
		WSACleanup();
		mIsRunning = false;
		mSelectThread->join();
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ReceivePacket Network::GetReceivePacket()
{
	std::lock_guard<std::mutex> lock(mPacketMutex);
	if (mReceivePacketQueue.empty())
	{
		return ReceivePacket{};
	}
	ReceivePacket receivePacket = mReceivePacketQueue.front();
	mReceivePacketQueue.pop();

	return receivePacket;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Network::Broadcast(const uint16 packetId, const char* bodyData, const int bodySize)
{
	for (auto& clientSession : mClientSessionManager->ClientVector())
	{
		if (!clientSession.IsConnect())
		{
			continue;
		}

		Send(clientSession.mIndex, packetId, bodyData, bodySize);
	}
}

void Network::Broadcast(const uint16 packetId, const char* bodyData, const int bodySize, const int exceptUserCount, ...)
{
	va_list ap;
	va_start(ap, exceptUserCount);

	for (auto& clientSession : mClientSessionManager->ClientVector())
	{
		if (!clientSession.IsConnect())
		{
			continue;
		}

		for (int i = 0; i < exceptUserCount; ++i)
		{
			uint64 uniqueId = va_arg(ap, uint64);
			if (clientSession.mUniqueId == uniqueId)
			{
				continue;
			}
		}

		Send(clientSession.mIndex, packetId, bodyData, bodySize);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NetworkLib::ErrorCode Network::Send(int32 sessionIndex, const uint16 packetId, const char* bodyData, const int bodySize)
{
	std::lock_guard<std::mutex> lock(mSessionMutex);

	ClientSession* session = mClientSessionManager->FindClientSession(sessionIndex);
	if (nullptr == session)
	{
		return ErrorCode::CLIENT_SESSION_NOT_CONNECTED;
	}

	return SendProcess(*session, packetId, bodyData, bodySize);
}

NetworkLib::ErrorCode Network::Send(uint64 sessionUniqueId, const uint16 packetId, const char* bodyData, const int bodySize)
{
	std::lock_guard<std::mutex> lock(mSessionMutex);

	ClientSession* session = mClientSessionManager->FindClientSession(sessionUniqueId);
	if (nullptr == session)
	{
		return ErrorCode::CLIENT_SESSION_NOT_CONNECTED;
	}

	return SendProcess(*session, packetId, bodyData, bodySize);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NetworkLib::ErrorCode Network::SendProcess(ClientSession& clientSession, const uint16 packetId, const char* bodyData, const int bodySize)
{
	uint16 totalSize = PACKET_HEADER_SIZE + bodySize;
	if (static_cast<uint32>(clientSession.mSendSize + totalSize) > mConfig->mMaxSessionBufferSize)
	{
		return ErrorCode::CLIENT_SESSION_SEND_BUFFER_IS_FULL;
	}

	PacketHeader header{ totalSize, packetId };
	memcpy_s(&clientSession.mSendBuffer[clientSession.mSendSize], PACKET_HEADER_SIZE, reinterpret_cast<char*>(&header), PACKET_HEADER_SIZE);
	clientSession.mSendSize += PACKET_HEADER_SIZE;

	if (bodySize > 0)
	{
		memcpy_s(&clientSession.mSendBuffer[clientSession.mSendSize], bodySize, bodyData, bodySize);
	}

	clientSession.mSendSize += bodySize;

	return SendClient(clientSession);
}