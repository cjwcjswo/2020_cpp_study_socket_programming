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
	Core::ErrorCode ReuseAddr();


public:
	Core::ErrorCode SetLingerMode();
	Core::ErrorCode SetNonBlockingMode();

	virtual Core::ErrorCode Create();
	virtual Core::ErrorCode Bind(const wchar* address, const uint16 portNum);
	virtual Core::ErrorCode Listen();
	virtual TCPSocket Accept();

	virtual int Send(char* sendBuffer, const int bufferSize);
	virtual int Receive(char* receiveBuffer, const int bufferSize);
	virtual void Close();
	virtual void Clear();

	inline SOCKET Socket() const { return mSocket; }
};

