#pragma once

#include "PrimitiveTypes.h"


namespace Core
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)
	struct PacketHeader
	{
		uint16 mPacketSize = 0;
		uint16 mPacketId = 0;
	};
#pragma pack(pop)

	//TODO 최흥배: 아래와 같이 상황에 따라서 변할 수 있는 것은 설정 파일을 Json으로 만들고, json을 읽어서 값을 사용하도록 합니다.	
	constexpr uint16 MAX_PACKET_BODY_SIZE = 1024;
	constexpr uint16 PACKET_HEADER_SIZE = sizeof(PacketHeader);

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	struct ReceivePacket
	{
		int32 mSessionIndex = -1;
		uint64 mSessionUniqueId = 0;
		uint16 mPacketId = 0;
		uint16 mBodyDataSize = 0;
		char* mBodyData = nullptr;
	};

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//TODO 최흥배: 학습을 위해서 사용한 것이라고 생각합니다. 다만 아래와 같은 상수는 특별한 이유가 없다면 const를 사용합니다.
	constexpr static int PACKET_ID_START = 10;
	constexpr static int PACKET_ID_END = 50;

	enum class PacketId
	{
		PACKET_START = PACKET_ID_START,
		//////////////////////////////////////////////////////////////
		CONNECT,
		DISCONNECT,

		//////////////////////////////////////////////////////////////
		PACKET_END = PACKET_ID_END,
	};
}