#include <iostream>
#include "NetworkCore.h"

using namespace std;

/*

		Private 영역

*/

ErrorCode NetworkCore::Init()
{
	WSADATA wsa_data;

	if (WSAStartup(MAKEWORD(2, 2), &wsa_data) == SOCKET_ERROR)
	{
		return ErrorCode::WSA_START_UP_FAIL;
	}

	this->accept_socket_ = socket(AF_INET, SOCK_STREAM, 0);
	if (this->accept_socket_ == INVALID_SOCKET)
	{
		return ErrorCode::SOCKET_INIT_FAIL;
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
		client_deque_.push_back(client_socket);

		cout << static_cast<int>(client_socket) << " connect" << endl;
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

ErrorCode NetworkCore::SelectClient(const fd_set& read_set, const fd_set& write_set)
{
	for (auto& client_socket : client_deque_)
	{
		char buf[BUFSIZ];
		int len = recv(client_socket, buf, BUFSIZ, 0);
		if (len == 0)
		{
			FD_CLR(client_socket, &this->read_set_);
			FD_CLR(client_socket, &this->write_set_);

			for (auto itr = client_deque_.begin(); itr != client_deque_.end(); ++itr)
			{
				client_deque_.erase(itr);
				break;
			}
			cout << client_socket << " disconnect" << endl;
		}
	}
}

/*

		Public 영역

*/

NetworkCore::NetworkCore() : accept_socket_(0), read_set_(), write_set_()
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
	if(error_code != ErrorCode::SUCCESS)
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

		error_code = this->SelectClient(read_set, write_set);
		if (error_code != ErrorCode::SUCCESS)
		{
			cout << static_cast<int>(error_code) << endl;
			continue;
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