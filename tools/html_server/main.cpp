#include <flame/foundation/network.h>

#include <iostream>

using namespace flame;
using namespace flame::network;

Server* server = nullptr;

int main(int argc, char **args)
{
	server = Server::create(SocketTcpRaw, 80, nullptr, [](void* id) {
		// connected
		server->set_client(id, [id](const std::string& msg) {
			auto lines = SUS::split(msg, '\n');
			auto sp = SUS::split(lines[0], ' ');
			sp[1].erase(sp[1].begin());
			std::filesystem::path path(sp[1]);
			if (path.empty())
				path = L"index.html";
			path = std::filesystem::current_path() / path;
			if (std::filesystem::exists(path))
			{
				auto content = get_file_content(path);

				std::string reply;
				reply += "HTTP/1.1 200 OK\r\n";
				reply += "Connection: close\r\n";
				reply += "Content-Type: text/html\r\n";
				reply += std::format("Content-Length: {}\r\n", (int)content.size());
				reply += "\r\n";
				reply += content;
				reply += "\r\n";

				server->send(id, reply);
			}
		}, []() {
			int cut = 1;
			// disconnected
		});
	});

	while (true)
	{
		;
	}

	return 0;
}
