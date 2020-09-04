#include "../NetworkLib/Network.h"
#include <thread>

using namespace NetworkLib;


int main()
{
	Network network;
	if (network.Init() != ErrorCode::SUCCESS)
	{
		return -1;
	}
	
	network.Run();

	while (true)
	{
		Sleep(1000);

		Packet packet = network.GetReceivePacket();
		if (packet.mPacketId == 0)
		{
			continue;
		}

		network.Send(packet.mSessionIndex, packet.mPacketId, packet.mBodyData, packet.mBodyDataSize);
	}

	return 0;
}