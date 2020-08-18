#pragma once
#pragma comment(lib,"ws2_32")

#include <WinSock2.h>

#include "PrimitiveTypes.h"
#include "ErrorCode.h"


class TCPSocket
{
public:
	explicit TCPSocket(SOCKET socket) : mSocket(socket) {};
	~TCPSocket() = default;


protected:
	SOCKET mSocket = INVALID_SOCKET;
	

protected:
	NetworkLib::ErrorCode ReuseAddr();


public:
	NetworkLib::ErrorCode SetLingerMode();
	NetworkLib::ErrorCode SetNonBlockingMode();

	virtual NetworkLib::ErrorCode Create();
	virtual NetworkLib::ErrorCode Bind(const wchar* address, const uint16 portNum);
	virtual NetworkLib::ErrorCode Listen();
	virtual TCPSocket Accept();

	virtual int Send(char* sendBuffer, const int bufferSize);
	virtual int Receive(char* receiveBuffer, const int bufferSize);
	virtual void Close();
	virtual void Clear();

	inline SOCKET Socket() const { return mSocket; }
};

