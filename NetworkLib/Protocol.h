#pragma once

#pragma pack(push, 1)
struct PacketHeader
{
	unsigned short packet_size = 0;
	unsigned short packet_id = 0;
};
#pragma pack(pop)

constexpr unsigned short kMaxPacketBodySize = 1024;
constexpr unsigned short kPacketHeaderSize = sizeof(PacketHeader);

struct ReceivePacket
{
	int session_index = -1;
	unsigned long session_unique_id = 0;
	unsigned short packet_id = 0;
	unsigned short body_data_size = 0;
	char* ptr_body_data = nullptr;
};