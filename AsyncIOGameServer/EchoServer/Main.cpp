#include "../NetworkLib/Network.h"


using namespace NetworkLib;


int main()
{
	Network network;
	if (network.Init(100) != ErrorCode::SUCCESS)
	{
		return -1;
	}
	
	network.Run();

	while (true)
	{
		Packet packet = network.GetReceivePacket();
		if (packet.mPacketId == 0)
		{
			continue;
		}

		network.Send(packet.mSessionIndex, packet.mPacketId, packet.mBodyData, packet.mBodyDataSize);
	}

	return 0;
}