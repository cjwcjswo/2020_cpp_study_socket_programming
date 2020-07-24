#include <iostream>
#include "NetworkCore.h"

using namespace std;

ErrorCode NetworkCore::Init()
{
	WSADATA wsa_data;

	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == SOCKET_ERROR)
	{
		return ErrorCode::WSA_START_UP_FAIL;
	}

	int addr_family = AF_INET;
	this->accept_socket_ = socket(addr_family, SOCK_STREAM, 0);
	if (this->accept_socket_ == INVALID_SOCKET)
	{
		return ErrorCode::SOCKET_INIT_FAIL;
	}

	SOCKADDR_IN socket_addr_in;
	int socket_addr_in_size = sizeof(socket_addr_in);
	ZeroMemory(&socket_addr_in, socket_addr_in_size);
	socket_addr_in.sin_family = addr_family;
	socket_addr_in.sin_port = htons(32452);
	socket_addr_in.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(this->accept_socket_, (SOCKADDR*)&socket_addr_in, socket_addr_in_size) == SOCKET_ERROR)
	{
		return ErrorCode::SOCKET_BIND_FAIL;
	}

	if (listen(this->accept_socket_, SOMAXCONN) == SOCKET_ERROR)
	{
		return ErrorCode::SOCKET_LISTEN_FAIL;
	}

	cout << "Init Ok" << endl;

	return ErrorCode::SUCCESS;
}


NetworkCore::NetworkCore() : accept_socket_(0)
{
}

NetworkCore::~NetworkCore()
{
	Stop();
}

ErrorCode NetworkCore::Start()
{
	cout << "Start" << endl;
	ErrorCode error_code = Init();
	if(error_code != ErrorCode::SUCCESS)
	{
		return error_code;
	}


	fd_set read_set, write_set, except_set;
	fd_set tmp_read_set, tmp_write_set, tmp_except_set;

	FD_ZERO(&read_set);
	FD_ZERO(&write_set);
	FD_ZERO(&except_set);

	FD_SET(this->accept_socket_, &read_set);
	FD_SET(this->accept_socket_, &except_set);

	tmp_read_set = read_set;
	tmp_write_set = write_set;
	tmp_except_set = except_set;

	while (true)
	{
		read_set = tmp_read_set;
		write_set = tmp_write_set;
		except_set = tmp_except_set;

		int result = select(NULL, &read_set, &write_set, &except_set, nullptr);
		if (result == 0)
		{
			cout << "NONE" << endl;
			continue;
		}
		if (result == SOCKET_ERROR)
		{
			return ErrorCode::SOCKET_SELECT_FAIL;
		}

		if (FD_ISSET(this->accept_socket_, &read_set))
		{
			SOCKADDR_IN socket_addr_in;
			int size = sizeof(socket_addr_in);
			SOCKET client_socket = accept(this->accept_socket_, (SOCKADDR*)&socket_addr_in, &size);
			if (client_socket != INVALID_SOCKET)
			{
				FD_SET(client_socket, &tmp_read_set);
				FD_SET(client_socket, &tmp_write_set);
				FD_SET(client_socket, &tmp_except_set);
				client_deque_.push_back(client_socket);

				cout << static_cast<int>(client_socket) << " connect" << endl;
			}
		}
		for (auto& client_socket : client_deque_)
		{
			char buf[BUFSIZ];
			int len = recv(client_socket, buf, BUFSIZ, 0);
			if (len == 0)
			{
				FD_CLR(client_socket, &tmp_read_set);
				FD_CLR(client_socket, &tmp_write_set);
				FD_CLR(client_socket, &tmp_except_set);

				for (auto itr = client_deque_.begin(); itr != client_deque_.end(); ++itr)
				{
					client_deque_.erase(itr);
					break;
				}
				cout << client_socket << " disconnect" << endl;
			}
		}
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