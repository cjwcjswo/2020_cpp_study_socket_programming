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

	return 0;
}