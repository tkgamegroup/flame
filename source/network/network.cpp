// MIT License
// 
// Copyright (c) 2018 wjs
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <flame/network/network.h>

#include <winsock2.h>
#include <assert.h>

namespace flame
{
	struct OneClientServerWebSocketPrivate : OneClientServerWebSocket
	{
		std::mutex mtx; // all ops

		int fd_s, fd_c;

		Function<void(void* c, int size, void* data)> message_callback;

		inline bool send(int size, void* data)
		{
			auto res = ::send(fd_c, (char*)data, size, 0);
			return res > 0;
		}
	};

	bool OneClientServerWebSocket::send(int size, void* data)
	{
		return ((OneClientServerWebSocketPrivate*)this)->send(size, data);
	}

	OneClientServerWebSocket* OneClientServerWebSocket::create(ushort port, int _timeout, const Function<void(void* c, int size, void* data)>& on_message)
	{
		int res;

		auto fd_s = socket(AF_INET, SOCK_STREAM, 0);
		assert(fd_s);

		sockaddr_in address = {};
		address.sin_family = AF_INET;
		address.sin_addr.S_un.S_addr = INADDR_ANY;
		address.sin_port = htons(port);
		res = bind(fd_s, (sockaddr*)& address, sizeof(address));
		assert(res == 0);
		res = listen(fd_s, 1);
		assert(res == 0);

		timeval timeout = { _timeout, 0 };
		fd_set rfds;
		FD_ZERO(&rfds);
		FD_SET(fd_s, &rfds);
		res = select(-1, &rfds, nullptr, nullptr, &timeout);
		if (res <= 0)
		{
			closesocket(fd_s);
			return nullptr;
		}

		auto fd_c = accept(fd_s, nullptr, nullptr);

		auto s = new OneClientServerWebSocketPrivate;
		s->fd_s = fd_s;
		s->fd_c = fd_c;
		s->message_callback = on_message;

		auto thiz = s;

		thread(Function<void(void*)>([](void* c) {
				auto thiz = *((OneClientServerWebSocketPrivate * *)c);
				while (true)
				{
					char buf[1024];
					auto ret = recv(thiz->fd_c, buf, 1024, 0);
					if (ret <= 0)
						return;

					if (ret > 3 &&
						buf[0] == 'G' &&
						buf[1] == 'E' &&
						buf[2] == 'T')
					{
						buf[ret] = 0;

						std::regex reg_key(R"(Sec-WebSocket-Key: ([\w\+\/]+))");

						std::string req(buf);
						auto lines = string_split(req, '\n');
						for (auto& l : lines)
						{
							std::smatch match;
							if (std::regex_search(l, match, reg_key))
							{
								std::string key = match[1];
								SHA1 sha1;
								sha1.update(key);
								sha1.update("258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
								key = base64_encode(sha1.final());

								char reply[1024], time_str[100];
								auto time = std::time(nullptr);
								std::strftime(time_str, 100, "%a, %d %b %Y %H:%M:%S GMT", std::localtime(&time));
								sprintf(reply, "HTTP/1.1 101 Switching Protocols\n"
												"Content-Length: 0\n"
												"Upgrade: websocket\n"
												"Sec-Websocket-Accept: %s\n"
												"Server: flame\n"
												"Connection: Upgrade\n"
												"Data: %s\n"
												"\n", key.c_str(), time_str);
								thiz->send(strlen(reply), reply);
							}
						}
					}
					else
						thiz->message_callback(ret, buf);
				}
			}, sizeof(void*), & thiz));

		return s;
	}

	void OneClientServerWebSocket::destroy(OneClientServerWebSocket* sock)
	{

	}

	void network_init()
	{
		WSADATA wsa_d = {};
		WSAStartup(MAKEWORD(1, 1), &wsa_d);
	}
}
