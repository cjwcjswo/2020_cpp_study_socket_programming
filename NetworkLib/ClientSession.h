#pragma once
#pragma comment(lib,"ws2_32")
#include <iostream>
#include <sstream>
#include <string>
#include <WinSock2.h>

class ClientSession
{
private:
	int index_ = -1;
	unsigned long unique_id_ = 0;
	SOCKET socket_ = INVALID_SOCKET;

public:
	explicit ClientSession(int index, unsigned long unique_id, SOCKET socket) : index_(index), unique_id_(unique_id), socket_(socket) {}

	inline int index() const { return this->index_; };
	inline unsigned long unique_id() const { return this->unique_id_; };
	inline SOCKET socket() const { return this->socket_; };
	inline void set_index(const int index) { this->index_ = index; };
	void set_unique_id(const unsigned long unique_id) { this->unique_id_ = unique_id; };
	void set_socket(const SOCKET& socket) { this->socket_ = socket; };

	std::string String() const;

public:
	constexpr static int kBuffSize = 1024;

	int remain_data_size = 0;
	int previous_receive_buff_pos_ = 0;
	int send_size = 0;
	char receive_buff_[kBuffSize] = { 0, };
	char send_buff_[kBuffSize] = { 0, };
};

