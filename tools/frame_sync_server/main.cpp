#include <flame/network/network.h>

using namespace flame;

int main(int argc, char **args)
{
	auto s = FrameSyncServer::create(SocketWeb, 5567, 2);
	while (true)
		std::this_thread::sleep_for(std::chrono::milliseconds(10000));

	return 0;
}
