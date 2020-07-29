#include <iostream>
#include "../../NetworkLib/NetworkCore.h"

int main()
{
    NetworkCore core;
    core.Run();
    while (true)
    {
        // Test Code
        Sleep(5000);
        std::cout << "Packet Check" << std::endl;
        core.GetReceivePacket();
    }
}