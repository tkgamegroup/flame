#include <flame/foundation/network.h>

using namespace flame;
using namespace flame::network;

Server* server = nullptr;

int main(int argc, char **args)
{
	server = Server::create(SocketTcp, 1975, nullptr, [](void* id) {
		server->set_client(id, [](std::string_view msg) {

		}, []() {
			printf("Disconnected.\n");
		});
		printf("Connected.\n");
	});

	printf("Waiting for connection.\n");

	while (true)
	{
		static char buf[1024];
		printf("cmd: ");
		scanf("%s", buf);
		std::string cmd(buf);

	}

	return 0;
}
