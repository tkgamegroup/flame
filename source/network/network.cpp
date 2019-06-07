// MIT License
// 
// Copyright (c) 2019 wjs
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
	bool web_socket_shakehank(int fd_c)
	{
		int res;
		fd_set rfds;
		timeval timeout = { 1, 0 };

		FD_ZERO(&rfds);
		FD_SET(fd_c, &rfds);
		res = select(-1, &rfds, nullptr, nullptr, &timeout);

		if (res <= 0)
		{
			closesocket(fd_c);
			return false;
		}

		uchar buf[1024 * 10];
		auto ret = recv(fd_c, (char*)buf, FLAME_ARRAYSIZE(buf), 0);

		auto p = buf;

		if (ret <= 0 || !(ret > 3 && p[0] == 'G' && p[1] == 'E' && p[2] == 'T'))
		{
			closesocket(fd_c);
			return false;
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

				char reply[1024 * 10], time_str[100];
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

		return true;
	}

	bool websocket_send(int fd, int size, void* data)
	{
		uchar buf[1024 * 100];
		auto p = buf;

		*p++ = 129;
		if (size <= 125)
			*p++ = size;
		else if (size <= 65535)
		{
			*p++ = 126;

			*p++ = size >> 8;
			*p++ = size & 0xff;
		}
		else
		{
			*p++ = 127;

			*p++ = size >> 56;
			*p++ = (size >> 48) & 0xff;
			*p++ = (size >> 40) & 0xff;
			*p++ = (size >> 32) & 0xff;

			*p++ = (size >> 24) >> 56;
			*p++ = (size >> 16) & 0xff;
			*p++ = (size >> 8) & 0xff;
			*p++ = size & 0xff;
		}

		memcpy(p, data, size);

		return ::send(fd, (char*)buf, int(p - buf) + size, 0) > 0;
	}

	int websocket_recv(uchar*& p, ulonglong& len, uint& mask_key)
	{
		uchar b1, b2;
		b1 = *p++;
		b2 = *p++;

		auto op = b1 & 0xf;
		auto mask = (b2 & 128) != 0;
		auto payload_len = b2 & 127;

		if (payload_len <= 125)
			len = payload_len;
		else if (payload_len == 126)
		{
			len = (*p++) << 8;
			len += *p++;
		}
		else if (payload_len == 127)
		{
			len = (*p++) << 56;
			len += (*p++) << 48;
			len += (*p++) << 40;
			len += (*p++) << 32;

			len += (*p++) << 24;
			len += (*p++) << 16;
			len += (*p++) << 8;
			len += *p++;
		}

		if (mask)
		{
			mask_key = *(uint*)p;
			p += sizeof(uint);
		}

		return op;
	}

	void websocket_recv_all(int fd, uchar* dst, int rest)
	{
		while (rest > 0)
		{
			auto ret = recv(fd, (char*)dst, rest, 0);
			dst += ret;
			rest -= ret;
		}
	}

	void websocket_mask(uchar* p, ulonglong len, uint mask_key)
	{
		for (auto i = 0; i < len; i++)
			p[i] ^= ((char*)& mask_key)[i % 4];
	}

	struct OneClientServerPrivate : OneClientServer
	{
		SocketType type;

		int fd_c;

		Function<void(void* c, int size, void* data)> message_callback;

		bool send(int size, void* data)
		{
			if (type == SocketWeb)
				return websocket_send(fd_c, size, data);
			else
				return ::send(fd_c, (char*)data, size, 0) > 0;
		}
	};

	bool OneClientServer::send(int size, void* data)
	{
		return ((OneClientServerPrivate*)this)->send(size, data);
	}

	OneClientServer* OneClientServer::create(SocketType type, ushort port, int _timeout, const Function<void(void* c, int size, void* data)>& on_message)
	{
		int res;

		auto fd_s = socket(AF_INET, SOCK_STREAM, 0);
		assert(fd_s != INVALID_SOCKET);

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
		closesocket(fd_s);

		if (type == SocketWeb)
		{
			if (!web_socket_shakehank(fd_c))
				return nullptr;
		}

		auto s = new OneClientServerPrivate;
		s->type = type;
		s->fd_c = fd_c;
		s->message_callback = on_message;
		s->ev_closed = CreateEvent(nullptr, true, false, nullptr);

		auto thiz = s;

		thread(Function<void(void*)>([](void* c) {
				auto thiz = *((OneClientServerPrivate * *)c);
				while (true)
				{
					uchar buf[1024 * 100];
					auto ret = recv(thiz->fd_c, (char*)buf, FLAME_ARRAYSIZE(buf), 0);
					if (ret <= 0)
					{
						SetEvent(thiz->ev_closed);
						return;
					}

					ulonglong len;
					uint mask_key;
					auto p = buf;
					auto op = websocket_recv(p, len, mask_key);
					websocket_recv_all(thiz->fd_c, buf + ret, (p - buf) + len - ret);
					websocket_mask(p, len, mask_key);
					p[len] = 0;

					if (op == 1)
						thiz->message_callback(len, p);
				}
			}, sizeof(void*), & thiz));

		return s;
	}

	void OneClientServer::destroy(OneClientServer* sock)
	{

	}

	struct FrameSyncServerPrivate : FrameSyncServer
	{
		SocketType type;

		int frame;
		int semaphore;

		std::vector<int> fd_cs;

		SerializableNode* frame_advance_data;

		bool send(int client_idx, int size, void* data)
		{
			if (type == SocketWeb)
				return websocket_send(fd_cs[client_idx], size, data);
			else
				return ::send(fd_cs[client_idx], (char*)data, size, 0) > 0;
		}
	};

	FrameSyncServer* FrameSyncServer::create(SocketType type, ushort port, int client_count)
	{
		int res;

		auto fd_s = socket(AF_INET, SOCK_STREAM, 0);
		assert(fd_s != INVALID_SOCKET);

		sockaddr_in address = {};
		address.sin_family = AF_INET;
		address.sin_addr.S_un.S_addr = INADDR_ANY;
		address.sin_port = htons(port);
		res = bind(fd_s, (sockaddr*)& address, sizeof(address));
		assert(res == 0);
		res = listen(fd_s, client_count);
		assert(res == 0);

		std::vector<int> fd_cs;
		while (fd_cs.size() < client_count)
		{
			auto fd_c = accept(fd_s, nullptr, nullptr);
			if (fd_c == INVALID_SOCKET)
				return nullptr;

			if (type == SocketWeb)
			{
				if (!web_socket_shakehank(fd_c))
					continue;
			}

			fd_cs.push_back(fd_c);
		}

		closesocket(fd_s);

		auto s = new FrameSyncServerPrivate;
		s->type = type;
		s->frame = 0;
		s->semaphore = 0;
		s->fd_cs = fd_cs;
		s->ev_closed = CreateEvent(nullptr, true, false, nullptr);
		s->frame_advance_data = SerializableNode::create("");
		s->frame_advance_data->new_attr("action", "frame");


		{
			srand(time(0));
			auto seed = ::rand();
			auto json = SerializableNode::create("");
			json->new_attr("action", "start");
			json->new_attr("seed", std::to_string(seed));
			auto str = json->to_string_json();
			for (auto i = 0; i < client_count; i++)
				s->send(i, str.size, str.v);
			SerializableNode::destroy(json);
		}

		auto thiz = s;

		thread(Function<void(void*)>([](void* c) {
				auto thiz = *((FrameSyncServerPrivate * *)c);

				fd_set rfds;

				while (true)
				{
					FD_ZERO(&rfds);
					for (auto fd : thiz->fd_cs)
						FD_SET(fd, &rfds);
					if (select(-1, &rfds, nullptr, nullptr, nullptr) > 0)
					{
						auto client_idx = 0;
						for (auto fd : thiz->fd_cs)
						{
							if (FD_ISSET(fd, &rfds))
							{
								uchar buf[1024 * 100];
								auto ret = recv(fd, (char*)buf, FLAME_ARRAYSIZE(buf), 0);
								if (ret > 0)
								{
									ulonglong len;
									uint mask_key;
									auto p = buf;
									auto op = websocket_recv(p, len, mask_key);
									websocket_recv_all(fd, buf + ret, (p - buf) + len - ret);
									websocket_mask(p, len, mask_key);
									p[len] = 0;

									if (op == 1)
									{
										auto json = SerializableNode::create_from_json_string((char*)p);
										auto n_frame = json->find_node("frame");
										if (n_frame && n_frame->type() == SerializableNode::Value)
										{
											auto frame = std::stoi(n_frame->value().c_str());
											if (frame == thiz->frame)
											{
												thiz->semaphore++;
												auto n_data = json->find_node("data");
												auto dst = thiz->frame_advance_data->new_node(std::to_string(client_idx + 1));
												for (auto i = 0; i < n_data->node_count(); i++)
												{
													auto n = json->node(i);
													dst->new_attr(n->name(), n->value());
												}

												if (thiz->semaphore >= thiz->fd_cs.size())
												{
													thiz->frame++;
													thiz->semaphore = 0;

													auto str = thiz->frame_advance_data->to_string_json();
													for (auto i = 0; i < 2; i++)
														thiz->send(i, str.size, str.v);
													SerializableNode::destroy(thiz->frame_advance_data);
													thiz->frame_advance_data = SerializableNode::create("");
													thiz->frame_advance_data->new_attr("action", "frame");
												}
											}
										}
										SerializableNode::destroy(json);
									}
								}
							}
							client_idx++;
						}
					}
				}
			}, sizeof(void*), & thiz));

		return s;
	}

	bool FrameSyncServer::send(int client_idx, int size, void* data)
	{
		return ((FrameSyncServerPrivate*)this)->send(client_idx, size, data);
	}

	void FrameSyncServer::destroy(FrameSyncServer* sock)
	{

	}

	void network_init()
	{
		WSADATA wsa_d = {};
		WSAStartup(MAKEWORD(1, 1), &wsa_d);
	}
}
