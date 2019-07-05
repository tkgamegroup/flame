#include <flame/network/network.h>

#include <winsock2.h>
#include <assert.h>

namespace flame
{
	void recv_all(int fd, uchar* dst, int rest)
	{
		while (rest > 0)
		{
			auto ret = recv(fd, (char*)dst, rest, 0);
			dst += ret;
			rest -= ret;
		}
	}

	bool websocket_shakehank(int fd_c)
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

		uchar buf[1024 * 16];
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

				char reply[1024 * 16], time_str[128];
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
		uchar buf[1024 * 64];
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

	std::vector<std::string> websocket_recv(int fd, bool& closed)
	{
		uchar buf[1024 * 64];

		auto bytes = recv(fd, (char*)buf, FLAME_ARRAYSIZE(buf), 0);
		if (bytes <= 0)
		{
			closed = true;
			return std::vector<std::string>();
		}

		std::vector<std::string> ret;

		auto p = buf;
		while (bytes > 0)
		{
			if (bytes < 2)
			{
				auto new_bytes = recv(fd, (char*)p + bytes, 2 - bytes, 0);
				if (new_bytes <= 0)
					break;
				bytes += new_bytes;
			}

			uchar b1, b2;
			b1 = *p++;
			b2 = *p++;
			bytes -= 2;

			auto op = b1 & 0xf;
			auto mask = (b2 & 128) != 0;
			auto payload_length = b2 & 127;

			auto need_bytes = 0;
			if (payload_length == 126)
				need_bytes = 2;
			else if (payload_length == 127)
				need_bytes = 8;
			if (mask)
				need_bytes += 4;

			if (bytes < need_bytes)
			{
				auto new_bytes = recv(fd, (char*)p + bytes, need_bytes - bytes, 0);
				if (new_bytes <= 0)
					break;
				bytes += new_bytes;
			}

			ulonglong length;

			if (payload_length <= 125)
				length = payload_length;
			else if (payload_length == 126)
			{
				length = (*p++) << 8;
				length += *p++;
				
				bytes -= 2;
			}
			else if (payload_length == 127)
			{
				length = (*p++) << 56;
				length += (*p++) << 48;
				length += (*p++) << 40;
				length += (*p++) << 32;

				length += (*p++) << 24;
				length += (*p++) << 16;
				length += (*p++) << 8;
				length += *p++;

				bytes -= 8;
			}

			uint mask_key;
			if (mask)
			{
				mask_key = *(uint*)p;

				p += 4;
				bytes -= 4;
			}

			if (bytes < length)
			{
				auto new_bytes = recv(fd, (char*)p + bytes, length - bytes, 0);
				if (new_bytes <= 0)
					break;
				bytes += new_bytes;
			}

			for (auto i = 0; i < length; i++)
				p[i] ^= ((char*)&mask_key)[i % 4];

			if (op == 1)
				ret.push_back(std::string(p, p + length));

			p += length;
			bytes -= length;
		}

		return ret;
	}

	struct OneClientServerPrivate : OneClientServer
	{
		SocketType type;

		int fd_c;

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

	OneClientServer* OneClientServer::create(SocketType type, ushort port, int _timeout, void on_message(void* c, const std::string& str), const Mail<>& capture)
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
			if (!websocket_shakehank(fd_c))
				return nullptr;
		}

		auto s = new OneClientServerPrivate;
		s->type = type;
		s->fd_c = fd_c;
		s->ev_closed = CreateEvent(nullptr, true, false, nullptr);

		std::thread([=]() {
			while (true)
			{
				bool closed = false;
				auto reqs = websocket_recv(fd_c, closed);
				for (auto& r : reqs)
					on_message(capture.p, r);
				if (closed)
				{
					SetEvent(s->ev_closed);
					delete_mail(capture);
					return;
				}
			}
		}).detach();

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
				if (!websocket_shakehank(fd_c))
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
				s->send(i, str.p->size(), str.p->data());
			delete_mail(str);
			SerializableNode::destroy(json);
		}

		std::thread([s]() {
			fd_set rfds;

			while (true)
			{
				FD_ZERO(&rfds);
				for (auto fd : s->fd_cs)
					FD_SET(fd, &rfds);
				if (select(-1, &rfds, nullptr, nullptr, nullptr) > 0)
				{
					auto client_idx = 0;
					for (auto fd : s->fd_cs)
					{
						if (FD_ISSET(fd, &rfds))
						{
							bool closed = false;
							auto reqs = websocket_recv(fd, closed);
							if (reqs.size() > 0)
							{
								auto json = SerializableNode::create_from_json_string(reqs[0]);
								auto n_frame = json->find_node("frame");
								if (n_frame && n_frame->type() == SerializableNode::Value)
								{
									auto frame = std::stoi(n_frame->value().c_str());
									if (frame == s->frame)
									{
										s->semaphore++;
										auto n_data = json->find_node("data");
										auto dst = s->frame_advance_data->new_node(std::to_string(client_idx + 1));
										for (auto i = 0; i < n_data->node_count(); i++)
										{
											auto n = json->node(i);
											dst->new_attr(n->name(), n->value());
										}

										if (s->semaphore >= s->fd_cs.size())
										{
											s->frame++;
											s->semaphore = 0;

											auto str = s->frame_advance_data->to_string_json();
											for (auto i = 0; i < 2; i++)
												s->send(i, str.p->size(), str.p->data());
											delete_mail(str);
											SerializableNode::destroy(s->frame_advance_data);
											s->frame_advance_data = SerializableNode::create("");
											s->frame_advance_data->new_attr("action", "frame");
										}
									}
								}
								SerializableNode::destroy(json);
							}
							if (closed)
							{
								// TODO
								/* do something! */
							}
						}
						client_idx++;
					}
				}
			}
		}).detach();

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
