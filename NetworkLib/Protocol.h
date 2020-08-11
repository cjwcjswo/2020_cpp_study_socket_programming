#pragma once

#include "PrimitiveTypes.h"
#include "Define.h"

namespace NetworkLib
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma pack(push, 1)
	struct PacketHeader
	{
		uint16 mPacketSize = 0;
		uint16 mPacketId = 0;
	};
#pragma pack(pop)

	constexpr uint16 PACKET_HEADER_SIZE = sizeof(PacketHeader);

	struct Packet
	{
		int32 mSessionIndex = INVALID_INDEX;
		uint64 mSessionUniqueId = INVALID_UNIQUE_ID;
		uint16 mPacketId = 0;
		uint16 mBodyDataSize = 0;
		char* mBodyData = nullptr;
	};

	// 적용 완료 (RAON 서버 코딩 스타일에서, cpp 파일의 함수 구현체부분은 주석으로 함수마다 구분을 해둬서 해당 부분은 남겨놓겠습니다)
	const uint16 PACKET_ID_START = 10;
	const uint16 PACKET_ID_END = 50;

	enum class PacketId : uint16
	{
		PACKET_START = PACKET_ID_START,

		CONNECT = 11,
		DISCONNECT = 12,

		PACKET_END = PACKET_ID_END,
	};
}