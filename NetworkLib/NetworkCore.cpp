#include "NetworkCore.h"
#include "ClientSessionManager.h"
#include "ClientSession.h"

using namespace Core;


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NetworkCore::~NetworkCore()
{
	Stop();
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkCore::LoadConfig()
{

}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode NetworkCore::Bind()
{
	SOCKADDR_IN socketAddrIn;
	int socketAddrInSize = sizeof(socketAddrIn);
	ZeroMemory(&socketAddrIn, socketAddrInSize);
	socketAddrIn.sin_family = AF_INET;
	socketAddrIn.sin_port = htons(32452);
	socketAddrIn.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (SOCKET_ERROR == bind(mAcceptSocket, (SOCKADDR*)&socketAddrIn, socketAddrInSize))
	{
		return ErrorCode::SOCKET_BIND_FAIL;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode NetworkCore::Listen()
{
	if (SOCKET_ERROR == listen(mAcceptSocket, SOMAXCONN))
	{
		return ErrorCode::SOCKET_LISTEN_FAIL;
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode NetworkCore::AcceptClient()
{
	SOCKADDR_IN socketAddrIn;
	int size = sizeof(socketAddrIn);
	SOCKET clientSocket = accept(mAcceptSocket, (SOCKADDR*)&socketAddrIn, &size);
	if (INVALID_SOCKET == clientSocket)
	{
		return ErrorCode::SOCKET_ACCEPT_CLIENT_FAIL;
	}

	ClientSession clientSession(ClientSession::INVALID_INDEX, 0, INVALID_SOCKET);
	clientSession.mSocket = clientSocket;

	clientSession.mIndex = mClientSessionManager->AllocClientSessionIndex();
	if (ClientSession::INVALID_INDEX == clientSession.mIndex)
	{
		CloseSession(ErrorCode::SOCKET_INDEX_POOL_IS_FULL, clientSession);
		return ErrorCode::SOCKET_INDEX_POOL_IS_FULL;
	}

	linger ling = { 0, 0 };
	setsockopt(clientSession.mSocket, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(ling));

	// Set Non-Blocking Mode (https://www.joinc.co.kr/w/man/4200/ioctlsocket)
	unsigned long mode = 1;
	if (SOCKET_ERROR == ioctlsocket(clientSession.mSocket, FIONBIO, &mode))
	{
		return ErrorCode::SOCKET_SET_FIONBIO_FAIL;
	}

	FD_SET(clientSession.mSocket, &mReadSet);
	FD_SET(clientSession.mSocket, &mWriteSet);
	
	clientSession.mUniqueId = mClientSessionManager->GenerateUniqueId();

	mClientSessionManager->ConnectClientSession(clientSession);

	PushReceivePacket(ReceivePacket{ clientSession.mIndex, clientSession.mUniqueId, static_cast<uint16>(PacketId::Connect), 0, nullptr});

	GLogger->PrintConsole(Color::GREEN, L"ConnectSession: %d / %d\n", clientSession.mIndex, clientSession.mUniqueId);

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode NetworkCore::CheckSelectResult(int selectResult)
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
void NetworkCore::SelectClient(const fd_set& readSet, const fd_set& writeSet)
{
	for (ClientSession& clientSession : mClientSessionManager->ClientVector())
	{
		if (!clientSession.IsConnect())
		{
			continue;
		}

		SOCKET clientSocket = clientSession.mSocket;
		if (FD_ISSET(clientSocket, &readSet))
		{
			ErrorCode errorCode = ReceiveClient(clientSession);
			if (ErrorCode::SUCCESS != errorCode)
			{
				CloseSession(errorCode, clientSession);
				continue;
			}
		}
		if (FD_ISSET(clientSocket, &writeSet))
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
ErrorCode NetworkCore::ReceiveClient(ClientSession& clientSession)
{
	int receivePos = 0;
	if (clientSession.mRemainDataSize > 0)
	{
		memcpy_s(clientSession.mReceiveBuffer, clientSession.mRemainDataSize, &clientSession.mReceiveBuffer[clientSession.mPreviousReceiveBufferPos], clientSession.mRemainDataSize);
		receivePos += clientSession.mRemainDataSize;
	}

	int length = recv(clientSession.mSocket, &clientSession.mReceiveBuffer[receivePos], ClientSession::BUFFER_SIZE, 0);
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

	memcpy_s(clientSession.mReceiveBuffer, clientSession.mRemainDataSize, &clientSession.mReceiveBuffer[clientSession.mPreviousReceiveBufferPos], clientSession.mRemainDataSize);
	clientSession.mRemainDataSize += length;
	int currentReceivePos = 0;
	PacketHeader* header;

	while ((clientSession.mRemainDataSize - currentReceivePos) >= PACKET_HEADER_SIZE)
	{
		header = reinterpret_cast<PacketHeader*>(&clientSession.mReceiveBuffer[currentReceivePos]);
		currentReceivePos += PACKET_HEADER_SIZE;
		uint16 requireBodySize = header->mPacketSize - PACKET_HEADER_SIZE;

		if (requireBodySize > 0)
		{
			if (requireBodySize > clientSession.mRemainDataSize - currentReceivePos)
			{
				currentReceivePos -= PACKET_HEADER_SIZE;
				break;
			}
			if (requireBodySize > MAX_PACKET_BODY_SIZE)
			{
				return ErrorCode::SOCKET_RECEIVE_MAX_PACKET_SIZE;
			}
		}

		ReceivePacket receivePacket = { clientSession.mIndex, clientSession.mUniqueId, header->mPacketId, requireBodySize, &clientSession.mReceiveBuffer[currentReceivePos] };
		PushReceivePacket(receivePacket);
		currentReceivePos += requireBodySize;
	}

	clientSession.mRemainDataSize -= currentReceivePos;
	clientSession.mPreviousReceiveBufferPos = currentReceivePos;

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkCore::SelectProcess()
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

		if (FD_ISSET(mAcceptSocket, &readSet))
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
ErrorCode NetworkCore::SendClient(ClientSession& clientSession)
{
	if (clientSession.mSendSize <= 0)
	{
		return ErrorCode::SUCCESS;
	}

	int length = send(clientSession.mSocket, clientSession.mSendBuffer, clientSession.mSendSize, 0);
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
void NetworkCore::PushReceivePacket(const ReceivePacket receivePacket)
{
	std::lock_guard<std::mutex> lock(mPacketMutex);
	mReceivePacketQueue.push(receivePacket);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void NetworkCore::CloseSession(const ErrorCode errorCode, const ClientSession& clientSession)
{
	GLogger->PrintConsole(Color::GREEN, L"CloseSession[%d]: %d / %d\n", static_cast<int>(errorCode), clientSession.mIndex, clientSession.mUniqueId);

	SOCKET clientSocket = clientSession.mSocket;
	if (!clientSession.IsConnect())
	{
		return;
	}

	closesocket(clientSocket);
	FD_CLR(clientSocket, &mReadSet);
	FD_CLR(clientSocket, &mWriteSet);

	if (ClientSession::INVALID_INDEX == clientSession.mIndex)
	{
		return;
	}

	PushReceivePacket(ReceivePacket{ clientSession.mIndex, clientSession.mUniqueId, static_cast<uint16>(PacketId::Disconnect), 0, nullptr });
	mClientSessionManager->DisconnectClientSession(clientSession.mIndex);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode NetworkCore::Init(const int maxSessionSize)
{
	WSADATA wsaData;

	if (SOCKET_ERROR == WSAStartup(MAKEWORD(2, 2), &wsaData))
	{
		return ErrorCode::WSA_START_UP_FAIL;
	}

	// Client Accept Socket Init
	mAcceptSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == mAcceptSocket)
	{
		return ErrorCode::SOCKET_INIT_FAIL;
	}

	// Reuse Address
	// https://www.joinc.co.kr/w/Site/Network_Programing/AdvancedComm/SocketOption
	char reuseAddr = 1;
	if (setsockopt(mAcceptSocket, SOL_SOCKET, SO_REUSEADDR, &reuseAddr, sizeof(reuseAddr)) < 0)
	{
		return ErrorCode::SOCKET_INIT_REUSE_ADDR_FAIL;
	}

	ErrorCode errorCode = Bind();
	if (ErrorCode::SUCCESS != errorCode)
	{
		return errorCode;
	}

	errorCode = Listen();
	if (ErrorCode::SUCCESS != errorCode)
	{
		return errorCode;
	}

	mClientSessionManager = new ClientSessionManager();
	mClientSessionManager->Init(maxSessionSize); // TODO: Load Config

	GLogger->PrintConsole(Color::GREEN, L"NetworkLib Init Success\n");

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode NetworkCore::Run()
{
	GLogger->PrintConsole(Color::GREEN, L"NetworkLib Run\n");

	FD_ZERO(&mReadSet);
	FD_ZERO(&mWriteSet);
	FD_SET(mAcceptSocket, &mReadSet);

	// Start Thread
	mIsRunning = true;
	mSelectThread = std::make_unique<std::thread>([&]() {SelectProcess(); });

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode NetworkCore::Stop()
{
	// Stop Accept Socket
	if (mAcceptSocket == INVALID_SOCKET)
	{
		return ErrorCode::SUCCESS;
	}
	shutdown(mAcceptSocket, SD_BOTH);
	closesocket(mAcceptSocket);

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
		
		GLogger->PrintConsole(Color::GREEN, L"NetworkLib Stop\n");
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ReceivePacket NetworkCore::GetReceivePacket()
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
void NetworkCore::Broadcast(const uint16 packetId, const char* bodyData, const int bodySize)
{
	std::lock_guard<std::mutex> lock(mSessionMutex);

	for (ClientSession clientSession : mClientSessionManager->ClientVector())
	{
		if (!clientSession.IsConnect())
		{
			continue;
		}

		Send(clientSession.mIndex, packetId, bodyData, bodySize);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Core::ErrorCode NetworkCore::Send(int32 sessionIndex, const uint16 packetId, const char* bodyData, const int bodySize)
{
	std::lock_guard<std::mutex> lock(mSessionMutex);

	ClientSession* session = mClientSessionManager->FindClientSession(sessionIndex);
	if (nullptr == session)
	{
		return ErrorCode::CLIENT_SESSION_NOT_CONNECTED;
	}

	return SendProcess(*session, packetId, bodyData, bodySize);
}

Core::ErrorCode NetworkCore::Send(uint64 sessionUniqueId, const uint16 packetId, const char* bodyData, const int bodySize)
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
Core::ErrorCode NetworkCore::SendProcess(ClientSession& clientSession, const uint16 packetId, const char* bodyData, const int bodySize)
{
	uint16 totalSize = PACKET_HEADER_SIZE + bodySize;
	if (clientSession.mSendSize + totalSize > ClientSession::BUFFER_SIZE)
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