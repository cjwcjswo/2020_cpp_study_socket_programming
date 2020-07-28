#include <iostream>
#include "NetworkCore.h"

using namespace std;

/*

		Private Zone

*/

ErrorCode NetworkCore::Init()
{
	WSADATA wsa_data;

	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == SOCKET_ERROR)
	{
		return ErrorCode::WSA_START_UP_FAIL;
	}

	// client accept socket init
	this->accept_socket_ = socket(AF_INET, SOCK_STREAM, 0);
	if (this->accept_socket_ == INVALID_SOCKET)
	{
		return ErrorCode::SOCKET_INIT_FAIL;
	}

	// reuse address
	// https://www.joinc.co.kr/w/Site/Network_Programing/AdvancedComm/SocketOption
	char reuse_addr = 1;
	if (setsockopt(this->accept_socket_, SOL_SOCKET, SO_REUSEADDR, &reuse_addr, sizeof(reuse_addr)) < 0)
	{
		return ErrorCode::SOCKET_INIT_REUSE_ADDR_FAIL;
	}

	ErrorCode error_code = this->Bind();
	if (error_code != ErrorCode::SUCCESS)
	{
		return error_code;
	}

	error_code = this->Listen();
	if (error_code != ErrorCode::SUCCESS)
	{
		return error_code;
	}

	cout << "Init Ok" << endl;

	return ErrorCode::SUCCESS;
}

ErrorCode NetworkCore::Bind()
{
	SOCKADDR_IN socket_addr_in;
	int socket_addr_in_size = sizeof(socket_addr_in);
	ZeroMemory(&socket_addr_in, socket_addr_in_size);
	socket_addr_in.sin_family = AF_INET;
	socket_addr_in.sin_port = htons(32452);
	socket_addr_in.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(this->accept_socket_, (SOCKADDR*)&socket_addr_in, socket_addr_in_size) == SOCKET_ERROR)
	{
		return ErrorCode::SOCKET_BIND_FAIL;
	}

	return ErrorCode::SUCCESS;
}

ErrorCode NetworkCore::Listen()
{
	if (listen(this->accept_socket_, SOMAXCONN) == SOCKET_ERROR)
	{
		return ErrorCode::SOCKET_LISTEN_FAIL;
	}

	return ErrorCode::SUCCESS;
}

ErrorCode NetworkCore::AcceptClient()
{
	SOCKADDR_IN socket_addr_in;
	int size = sizeof(socket_addr_in);
	SOCKET client_socket = accept(this->accept_socket_, (SOCKADDR*)&socket_addr_in, &size);
	if (client_socket != INVALID_SOCKET)
	{
		FD_SET(client_socket, &this->read_set_);
		FD_SET(client_socket, &this->write_set_);

		linger ling = { 0, 0 };
		setsockopt(client_socket, SOL_SOCKET, SO_LINGER, (char*)&ling, sizeof(ling));

		// set non-blocking mode (https://www.joinc.co.kr/w/man/4200/ioctlsocket)
		unsigned long mode = 1;
		if (ioctlsocket(client_socket, FIONBIO, &mode) == SOCKET_ERROR)
		{
			return ErrorCode::SOCKET_SET_FIONBIO_FAIL;
		}

		this->client_session_manager.ConnectClientSession(client_socket);

		return ErrorCode::SUCCESS;
	}

	return ErrorCode::SOCKET_ACCEPT_CLIENT_FAIL;
}

ErrorCode NetworkCore::CheckSelectResult(int select_result)
{
	if (select_result == 0)
	{
		return ErrorCode::SOCKET_SELECT_RESULT_ZERO;
	}

	if (select_result < 0)
	{
		return ErrorCode::SOCKET_SELECT_FAIL;
	}

	return ErrorCode::SUCCESS;
}

// Select Client Session
void NetworkCore::SelectClient(const fd_set& read_set, const fd_set& write_set)
{
	for (auto client_session : this->client_session_manager.client_deque())
	{
		SOCKET client_socket = client_session->socket();
		if (FD_ISSET(client_socket, &read_set))
		{
			ErrorCode error_code = this->ReceiveClient(client_session, read_set);
			if (error_code != ErrorCode::SUCCESS)
			{
				this->CloseSession(error_code, client_session);
			}
		}
		if (FD_ISSET(client_socket, &write_set))
		{
			ErrorCode error_code = this->SendClient(client_session, write_set);
			if (error_code != ErrorCode::SUCCESS)
			{
				this->CloseSession(error_code, client_session);
			}
		}
	}
}

ErrorCode NetworkCore::ReceiveClient(SharedPtrClientSession client_session, const fd_set& read_set)
{
	int receive_pos = 0;
	if (client_session->remain_data_size > 0)
	{
		memcpy_s(client_session->receive_buff_, client_session->remain_data_size, &client_session->receive_buff_[client_session->previous_receive_buff_pos_], client_session->remain_data_size);
		receive_pos += client_session->remain_data_size;
	}

	int length = recv(client_session->socket(), &client_session->receive_buff_[receive_pos], ClientSession::kBuffSize, 0);
	if (length == 0)
	{
		return ErrorCode::SOCKET_RECEIVE_ZERO;
	}
	if (length < 0)
	{
		auto net_error_code = WSAGetLastError();
		if (net_error_code == WSAEWOULDBLOCK)
		{
			return ErrorCode::SUCCESS;
		}
		else
		{
			return ErrorCode::SOCKET_RECEIVE_FAIL;
		}
	}

	client_session->remain_data_size += length;
	int current_receive_pos = 0;
	PacketHeader* header_ptr;

	while ((client_session->remain_data_size - current_receive_pos) >= kPacketHeaderSize)
	{
		header_ptr = reinterpret_cast<PacketHeader*>(&client_session->receive_buff_[current_receive_pos]);
		current_receive_pos += kPacketHeaderSize;
		unsigned short require_body_size = header_ptr->packet_size - kPacketHeaderSize;

		if (require_body_size > 0)
		{
			if (require_body_size > client_session->remain_data_size - current_receive_pos)
			{
				current_receive_pos -= kPacketHeaderSize;
				break;
			}
			if (require_body_size > kMaxPacketBodySize)
			{
				return ErrorCode::SOCKET_RECEIVE_MAX_PACKET_SIZE;
			}
		}

		ReceivePacket receive_packet = { client_session->index(), client_session->unique_id(), header_ptr->packet_id, require_body_size, &client_session->receive_buff_[current_receive_pos]};
		this->receive_packet_queue_.push(receive_packet);
		current_receive_pos += require_body_size;
	}

	client_session->remain_data_size -= current_receive_pos;
	client_session->previous_receive_buff_pos_ = current_receive_pos;

	return ErrorCode::SUCCESS;
}

ErrorCode NetworkCore::SendClient(SharedPtrClientSession client_session, const fd_set& write_set)
{
	if (client_session->send_size <= 0)
	{
		return ErrorCode::SUCCESS;
	}

	int length = send(client_session->socket(), client_session->send_buff_, client_session->send_size, 0);
	if (length <= 0)
	{
		return ErrorCode::SOCKET_SEND_SIZE_ZERO;
	}

	if (client_session->send_size > length)
	{
		auto remain = client_session->send_size - length;
		memmove_s(client_session->send_buff_, remain, &client_session->send_buff_[length], remain);
		client_session->send_size -= length;
	}
	else
	{
		client_session->send_size = 0;
	}

	return ErrorCode::SUCCESS;
}

void NetworkCore::CloseSession(const ErrorCode error_code, const SharedPtrClientSession client_session)
{
	std::cout << "[" << static_cast<int>(error_code) << "]: " << client_session->String() <<  std::endl;
	SOCKET client_socket = client_session->socket();
	this->client_session_manager.DisconnectClientSession(client_socket);
	closesocket(client_socket);
	FD_CLR(client_socket, &this->read_set_);
	FD_CLR(client_socket, &this->write_set_);
}

/*

		Public Zone

*/

NetworkCore::NetworkCore()
{
}

NetworkCore::~NetworkCore()
{
	Stop();
}

ErrorCode NetworkCore::Run()
{
	cout << "Start" << endl;
	ErrorCode error_code = Init();
	if (error_code != ErrorCode::SUCCESS)
	{
		return error_code;
	}


	FD_ZERO(&this->read_set_);
	FD_ZERO(&this->write_set_);
	FD_SET(this->accept_socket_, &this->read_set_);


	while (true)
	{
		auto read_set = this->read_set_;
		auto write_set = this->write_set_;

		// Block
		int select_result = select(NULL, &read_set, &write_set, nullptr, nullptr);

		error_code = this->CheckSelectResult(select_result);
		if (error_code != ErrorCode::SUCCESS)
		{
			cout << static_cast<int>(error_code) << endl;
			continue;
		}

		if (FD_ISSET(this->accept_socket_, &read_set))
		{
			error_code = this->AcceptClient();
			if (error_code != ErrorCode::SUCCESS)
			{
				cout << static_cast<int>(error_code) << endl;
				continue;
			}
		}

		this->SelectClient(read_set, write_set);
	}

	return ErrorCode::SUCCESS;
}

ErrorCode NetworkCore::Stop()
{
	// Stop Accept Socket
	shutdown(this->accept_socket_, SD_BOTH);
	closesocket(this->accept_socket_);
	WSACleanup();

	cout << "Stop Server";

	return ErrorCode::SUCCESS;
}

void NetworkCore::LoadConfig()
{

}