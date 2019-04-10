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

		address = {};
		int address_length = sizeof(address);
		auto fd_c = accept(fd_s, (sockaddr*)& address, &address_length);

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
					buf[ret] = 0;

					std::string req(buf);

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
