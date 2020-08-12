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

	PushReceivePacket(Packet{ clientSession.mIndex, clientSession.mUniqueId, static_cast<uint16>(PacketId::CONNECT), 0, nullptr });

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
void Network::SelectClient(const fd_set& readSet)
{
	mClientSessionManager->Select([&](ClientSession& session) {
		if (!session.IsConnect())
		{
			return;
		}

		TCPSocket clientSocket = session.mSocket;
		if (FD_ISSET(clientSocket.Socket(), &readSet))
		{
			ErrorCode errorCode = ReceiveClient(session);
			if (ErrorCode::SUCCESS != errorCode)
			{
				CloseSession(errorCode, session);
			}
		}
		}
	);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Network::ReceiveClient(ClientSession& clientSession)
{
	int length = clientSession.mSocket.Receive(clientSession.mReceiveBuffer, mConfig->mMaxSessionBufferSize);
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

	if (false == clientSession.mMessageBuffer.Push(clientSession.mReceiveBuffer, static_cast<size_t>(length)))
	{
		return ErrorCode::CLIENT_SESSION_MESSAGE_BUFFER_IS_FULL;
	}

	PacketHeader* header;
	while (clientSession.mMessageBuffer.DataSize() >= PACKET_HEADER_SIZE)
	{
		header = reinterpret_cast<PacketHeader*>(clientSession.mMessageBuffer.FrontData());
		uint16 requireBodySize = header->mPacketSize - PACKET_HEADER_SIZE;

		if (requireBodySize > 0)
		{
			if (requireBodySize > clientSession.mMessageBuffer.DataSize())
			{
				break;
			}
			if (requireBodySize > mConfig->mMaxPacketBodySize)
			{
				return ErrorCode::SOCKET_RECEIVE_MAX_PACKET_SIZE;
			}
		}

		//TODO 최흥배. TODO 중에 가장 뒤에 구현하는 것으로 하죠. 작업 시간이 좀 걸릴 수 있으니
		//받은 데이터를 패킷처리 스레드로 메모리 주소만 넘기고 있음.
		//받기 버퍼가 작아서 패킷처리에서 조금이라도 느려지면 덮어 쓸수 있을 것 같음. 더 좋은 방법을 생각해보자
		// 적용 완료(링버퍼)
		Packet receivePacket = { clientSession.mIndex, clientSession.mUniqueId, header->mPacketId, 0, nullptr };
		if (requireBodySize > 0)
		{
			receivePacket.mBodyDataSize = requireBodySize;
			receivePacket.mBodyData = clientSession.mMessageBuffer.FrontData() + PACKET_HEADER_SIZE;
		}

		clientSession.mMessageBuffer.Pop(header->mPacketSize);
		PushReceivePacket(receivePacket);
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Network::SelectProcess()
{
	ErrorCode errorCode;
	while (mIsRunning)
	{
		fd_set readSet = mReadSet;
				
		// Block
		int selectResult = select(NULL, &readSet, nullptr, nullptr, nullptr);

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

		SelectClient(readSet);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
ErrorCode Network::SendClient(ClientSession& clientSession)
{
	size_t dataSize = clientSession.mSendBuffer.DataSize();
	if (dataSize <= 0)
	{
		return ErrorCode::SUCCESS;
	}

	int length = clientSession.mSocket.Send(clientSession.mSendBuffer.FrontData(), static_cast<int>(dataSize));
	if (length <= 0)
	{
		return ErrorCode::SOCKET_SEND_SIZE_ZERO;
	}

	clientSession.mSendBuffer.Pop(static_cast<size_t>(length));
	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Network::PushReceivePacket(const Packet receivePacket)
{
	std::lock_guard<std::mutex> lock(mReceivePacketMutex);
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

	PushReceivePacket(Packet{ clientSession.mIndex, clientSession.mUniqueId, static_cast<uint16>(PacketId::DISCONNECT), 0, nullptr });
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
	mAcceptSocket = new TCPSocket{ INVALID_SOCKET };
	errorCode = mAcceptSocket->Create();
	if (ErrorCode::SUCCESS != errorCode)
	{
		return errorCode;
	}

	std::wstring wideIPAddress;
	wideIPAddress.assign(mConfig->mIPAddress.begin(), mConfig->mIPAddress.end());
	errorCode = mAcceptSocket->Bind(wideIPAddress.c_str(), mConfig->mPortNum);
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
	mSelectThread = std::make_unique<std::thread>([&]() {SelectProcess(); });
	mSendThread = std::make_unique<std::thread>([&]() {SendProcess(); });

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
		mClientSessionManager->CloseAll([&](ClientSession& session)
			{
				if (session.IsConnect())
				{
					CloseSession(ErrorCode::SUCCESS, session);
				}
			}
		);
		WSACleanup();
		mIsRunning = false;
		mSelectThread->join();
		mSendThread->join();
	}

	return ErrorCode::SUCCESS;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Packet Network::GetReceivePacket()
{
	std::lock_guard<std::mutex> lock(mReceivePacketMutex);
	if (mReceivePacketQueue.empty())
	{
		return Packet{};
	}
	Packet receivePacket = mReceivePacketQueue.front();
	mReceivePacketQueue.pop();

	return receivePacket;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Network::Broadcast(const uint16 packetId, char* bodyData, const uint16 bodySize)
{
	mClientSessionManager->SendAll([&](ClientSession& session)
		{
			if (!session.IsConnect())
			{
				return;
			}

			Send(session.mIndex, packetId, bodyData, bodySize);
		}
	);
}

//TODO: 최흥배 const int exceptUserCount, ... 대신 안전하게  initializer_list를 사용하도록 하죠
// 적용 완료
void Network::Broadcast(const uint16 packetId, char* bodyData, const uint16 bodySize, std::initializer_list<const uint64> exceptUserList)
{
	mClientSessionManager->SendAll([&](ClientSession& session)
		{
			if (!session.IsConnect())
			{
				return;
			}

			for (auto uniqueId : exceptUserList)
			{
				if (session.mUniqueId == uniqueId)
				{
					return;
				}
			}

			Send(session.mIndex, packetId, bodyData, bodySize);
		}
	);
}

//TODO 최흥배: 버그 아닌가요? bodyData는 스택에 있는 데이터라서 scope를 벗어나면 날라갑니다.
// 적용완료, 혹시 더 깔끔한 방법이 있는지..
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Network::Send(const int32 sessionIndex, const uint16 packetId, char* bodyData, const uint16 bodySize)
{
	std::lock_guard<std::mutex> lock(mSendPacketMutex);
	Packet sendPacket{sessionIndex, INVALID_UNIQUE_ID, packetId, bodySize, nullptr };
	sendPacket.mBodyData = new char[bodySize];
	memcpy_s(sendPacket.mBodyData, bodySize, bodyData, bodySize);
	mSendPacketQueue.push(sendPacket);
}

void Network::Send(const uint64 sessionUniqueId, const uint16 packetId, char* bodyData, const uint16 bodySize)
{
	std::lock_guard<std::mutex> lock(mSendPacketMutex);
	Packet sendPacket{ INVALID_INDEX, sessionUniqueId, packetId, bodySize, nullptr };
	sendPacket.mBodyData = new char[bodySize];
	memcpy_s(sendPacket.mBodyData, bodySize, bodyData, bodySize);
	mSendPacketQueue.push(sendPacket);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Network::SendProcess()
{
	while (mIsRunning)
	{
		std::unique_lock<std::mutex> lock(mSendPacketMutex);
		if (mSendPacketQueue.empty())
		{
			lock.unlock();
			std::this_thread::sleep_for(std::chrono::milliseconds(mConfig->mSendPacketCheckTick));
			continue;
		}

		Packet packet = mSendPacketQueue.front();
		mSendPacketQueue.pop();

		ClientSession* session = nullptr;
		if (INVALID_INDEX != packet.mSessionIndex)
		{
			session = mClientSessionManager->FindClientSession(packet.mSessionIndex);
		}
		else if (INVALID_UNIQUE_ID != packet.mSessionUniqueId)
		{
			session = mClientSessionManager->FindClientSession(packet.mSessionUniqueId);
		}

		if (nullptr == session)
		{
			continue;
		}

		uint16 totalSize = PACKET_HEADER_SIZE + packet.mBodyDataSize;
		if (static_cast<size_t>(totalSize) > session->mSendBuffer.RemainBufferSize())
		{
			// Client Session Send Buffer is full
			continue;
		}

		PacketHeader header{ totalSize, packet.mPacketId };
		session->mSendBuffer.Push(reinterpret_cast<char*>(&header), PACKET_HEADER_SIZE);
		if (packet.mBodyDataSize > 0)
		{
			session->mSendBuffer.Push(packet.mBodyData, static_cast<size_t>(packet.mBodyDataSize));
		}

		SendClient(*session);

		delete[] packet.mBodyData;
	}
}