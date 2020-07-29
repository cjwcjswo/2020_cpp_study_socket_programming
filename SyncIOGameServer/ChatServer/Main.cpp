#include <iostream>
#include "../../NetworkLib/NetworkCore.h"

int main()
{
    NetworkCore core;
    core.Run();
    while (true)
    {
        // Test Code
        Core::ReceivePacket receivePacket = core.GetReceivePacket();
        if (0 != receivePacket.mPacketId)
        {
            std::cout << "[PacketId: " << receivePacket.mPacketId << "] Index: " << receivePacket.mSessionIndex << ", " << receivePacket.mSessionUniqueId << std::endl;
        }
    }
}