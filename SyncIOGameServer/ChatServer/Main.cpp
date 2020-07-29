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
            GLogger->PrintConsole(Color::YELLOW, L"[PacketId: %d] SessionIndex: %d, SessionUniqueId: %d\n",
                receivePacket.mPacketId, receivePacket.mSessionIndex, receivePacket.mSessionUniqueId);
        }
    }
}