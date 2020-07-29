#include <iostream>
#include "ChatServer.h"

int main()
{
    ChatServer server;
    if (CS::ErrorCode::SUCCESS != server.Init())
    {
        return -1;
    }
    if (CS::ErrorCode::SUCCESS != server.Run())
    {
        return -1;
    }

    return 0;
}