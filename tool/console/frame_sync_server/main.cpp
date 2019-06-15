#include <flame/network/network.h>

using namespace flame;

int main(int argc, char **args)
{
	network_init();

	auto s = FrameSyncServer::create(SocketWeb, 5567, 2);
	wait_for(s->ev_closed);

	return 0;
}
