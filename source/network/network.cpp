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
			uchar buf[1024];

			auto p = buf;

			*p++ = 130;
			if (size <= 125)
				* p++ = size;
			else if (size <= 65535)
			{
				*p++ = 126;
				*(ushort*)p = size;
				p += sizeof(ushort);
			}
			else
			{
				*p++ = 127;
				*(ulonglong*)p = size;
				p += sizeof(ulonglong);
			}

			memcpy(p, data, size);

			auto res = ::send(fd_c, (char*)buf, 2 + size, 0);
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

		FD_ZERO(&rfds);
		FD_SET(fd_c, &rfds);
		res = select(-1, &rfds, nullptr, nullptr, &timeout);
		if (res <= 0)
		{
			closesocket(fd_s);
			closesocket(fd_c);
			return nullptr;
		}

		uchar buf[1024];
		auto ret = recv(fd_c, (char*)buf, FLAME_ARRAYSIZE(buf), 0);

		auto p = buf;

		if (ret <= 0 || !(ret > 3 && p[0] == 'G' && p[1] == 'E' && p[2] == 'T'))
		{
			closesocket(fd_s);
			closesocket(fd_c);
			return nullptr;
		}

		p[ret] = 0;

		std::regex reg_key(R"(Sec-WebSocket-Key: (.*))");

		std::string req((char*)p);
		auto lines = string_split(req, '\n');
		for (auto& l : lines)
		{
			std::smatch match;
			if (std::regex_search(l, match, reg_key))
			{
				std::string key = match[1];
				SHA1 sha1;
				sha1.update(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
				key = base64_encode(sha1.final_bin());

				char reply[1024], time_str[100];
				auto time = std::time(nullptr);
				std::strftime(time_str, FLAME_ARRAYSIZE(time_str), "%a, %d %b %Y %H:%M:%S GMT", std::localtime(&time));
				sprintf(reply, "HTTP/1.1 101 Switching Protocols\r\n"
					"Content-Length: 0\r\n"
					"Upgrade: websocket\r\n"
					"Sec-Websocket-Accept: %s\r\n"
					"Server: flame\r\n"
					"Connection: Upgrade\r\n"
					"Data: %s\r\n"
					"\r\n", key.c_str(), time_str);
				auto res = ::send(fd_c, reply, strlen(reply), 0);
				assert(res > 0);
			}
		}

		auto s = new OneClientServerWebSocketPrivate;
		s->fd_s = fd_s;
		s->fd_c = fd_c;
		s->message_callback = on_message;

		auto thiz = s;

		thread(Function<void(void*)>([](void* c) {
				auto thiz = *((OneClientServerWebSocketPrivate * *)c);
				while (true)
				{
					uchar buf[1024];
					auto ret = recv(thiz->fd_c, (char*)buf, FLAME_ARRAYSIZE(buf), 0);
					if (ret <= 0)
						return;

					auto p = buf;

					uchar b1, b2;
					b1 = *p++;
					b2 = *p++;

					auto mask = (b2 & 128) != 0;
					auto payload_len = b2 & 127;

					ulonglong len;
					if (payload_len <= 125)
						len = payload_len;
					else if (payload_len == 126)
					{
						len = *(ushort*)p;
						p += sizeof(ushort);
					}
					else if (payload_len == 127)
					{
						len = *(ulonglong*)p;
						p += sizeof(ulonglong);
					}

					uint mask_key;
					if (mask)
					{
						mask_key = *(uint*)p;
						p += sizeof(uint);
					}

					if (mask)
					{
						for (auto i = 0; i < len; i++)
							p[i] = p[i] ^ ((char*)& mask_key)[i % 4];
					}

					thiz->message_callback(len, p);
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
