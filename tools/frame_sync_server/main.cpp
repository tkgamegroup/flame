#include <flame/network/network.h>

using namespace flame;

int main(int argc, char **args)
{
	auto s = FrameSyncServer::create(SocketWeb, 5567, 2);
	while (true)
		sleep(10000);

	return 0;
}
