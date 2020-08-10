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
//TODO 최흥배
//람다를 사용하여 아래 함수의 내용을 mClientSessionManager에 람다로 전달해보자
//가능하면 Network 클래스는 mClientSessionManager의 클라이언트 객체를 순회하지 않도록 하자.
// 지금의 경우 ClientVector() 구현 방법이 바뀌면 SelectClient도 같이 바뀌어야 한다.
// 적용 완료
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

		//TODO 최흥배. TODO 중에 가장 뒤에 구현하는 것으로 하죠. 작업 시간이 좀 걸릴 수 있으니
		//받은 데이터를 패킷처리 스레드로 메모리 주소만 넘기고 있음.
		//받기 버퍼가 작아서 패킷처리에서 조금이라도 느려지면 덮어 쓸수 있을 것 같음. 더 좋은 방법을 생각해보자
		Packet receivePacket = { clientSession.mIndex, clientSession.mUniqueId, header->mPacketId, 0, nullptr };
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
		//TODO 최흥배.
		//mReadSet 공간의 사이에 빈 공간이 없게 해서 현재 연결된 클라이언트만 조사하도록 한다.
		fd_set readSet = mReadSet;

		//TODO 최흥배.
		//write 이벤트는 조사하지 않는다. write는 send용 스레드를 만들어서 이 스레드에서 주기적으로 send 한다
		// 적용 완료

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
	// TODO 최진우: 브로드캐스트는 패킷 전송에 한번에 처리 할 수 있도록 수정
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

void Network::Broadcast(const uint16 packetId, char* bodyData, const uint16 bodySize, const int exceptUserCount, ...)
{
	// TODO 최진우: 브로드캐스트는 패킷 전송에 한번에 처리 할 수 있도록 수정
	va_list ap;
	va_start(ap, exceptUserCount);
	std::vector<uint64> exceptUniqueIdVector;

	for (int i = 0; i < exceptUserCount; ++i)
	{
		uint64 uniqueId = va_arg(ap, uint64);
		exceptUniqueIdVector.push_back(uniqueId);
	}

	mClientSessionManager->SendAll([&](ClientSession& session)
		{
			if (!session.IsConnect())
			{
				return;
			}

			for (auto uniqueId : exceptUniqueIdVector)
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Network::Send(int32 sessionIndex, const uint16 packetId, char* bodyData, const uint16 bodySize)
{
	std::lock_guard<std::mutex> lock(mSendPacketMutex);
	mSendPacketQueue.push(Packet{ sessionIndex, INVALID_UNIQUE_ID, packetId, bodySize, bodyData });
}

void Network::Send(uint64 sessionUniqueId, const uint16 packetId, char* bodyData, const uint16 bodySize)
{
	std::lock_guard<std::mutex> lock(mSendPacketMutex);
	mSendPacketQueue.push(Packet{INVALID_INDEX, sessionUniqueId, packetId, bodySize, bodyData});
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void Network::SendProcess()
{
	while (mIsRunning)
	{
		std::lock_guard<std::mutex> lock(mSendPacketMutex);
		if (mSendPacketQueue.empty())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(mConfig->mSendPacketCheckTick));
			continue;
		}

		Packet sendPacket = mSendPacketQueue.front();
		mSendPacketQueue.pop();

		ClientSession* session = nullptr;
		if (INVALID_INDEX != sendPacket.mSessionIndex)
		{
			session = mClientSessionManager->FindClientSession(sendPacket.mSessionIndex);
		}
		else if (INVALID_UNIQUE_ID != sendPacket.mSessionUniqueId)
		{
			session = mClientSessionManager->FindClientSession(sendPacket.mSessionUniqueId);
		}

		if (nullptr == session)
		{
			continue;
		}

		uint16 totalSize = PACKET_HEADER_SIZE + sendPacket.mBodyDataSize;
		if (static_cast<uint32>(session->mSendSize + totalSize) > mConfig->mMaxSessionBufferSize)
		{
			// Client Session Send Buffer is full
			continue;
		}

		PacketHeader header{ totalSize, sendPacket.mPacketId };
		memcpy_s(&session->mSendBuffer[session->mSendSize], PACKET_HEADER_SIZE, reinterpret_cast<char*>(&header), PACKET_HEADER_SIZE);
		session->mSendSize += PACKET_HEADER_SIZE;

		if (sendPacket.mBodyDataSize > 0)
		{
			memcpy_s(&session->mSendBuffer[session->mSendSize], sendPacket.mBodyDataSize, sendPacket.mBodyData, sendPacket.mBodyDataSize);
		}

		session->mSendSize += sendPacket.mBodyDataSize;

		SendClient(*session);
	}
}