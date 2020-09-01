#pragma once

#include "PrimitiveTypes.h"
#include "Define.h"

namespace NetworkLib
{
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	const int MAX_BODY_SIZE = 1024;

#pragma pack(push, 1)
	struct PacketHeader
	{
		uint16 mPacketSize = 0;
		uint16 mPacketId = 0;
	};

	struct PacketData
	{
		PacketHeader mHeader;
		char mBodyData[MAX_BODY_SIZE] = { 0, };
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