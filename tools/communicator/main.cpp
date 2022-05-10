#include <flame/foundation/network.h>

#include <iostream>

using namespace flame;
using namespace flame::network;

Server* server = nullptr;
void* id = nullptr;

int main(int argc, char **args)
{
	server = Server::create(SocketTcp, 1975, nullptr, [](void* _id) {
		id = _id;
		server->set_client(id, [](std::string_view msg) {

		}, []() {
			printf("\nDisconnected.\n");
		});
		printf("\nConnected.\n");
	});

	printf("Waiting for connection.\n");

	while (true)
	{
		printf("cmd: ");
		std::string cmd;
		std::getline(std::cin, cmd);
		if (id)
			server->send(id, cmd);
	}

	return 0;
}
