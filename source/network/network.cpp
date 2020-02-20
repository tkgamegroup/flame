#include <flame/serialize.h>
#include <flame/network/network.h>

#include <winsock2.h>
#include <assert.h>

namespace flame
{
	bool websocket_shakehank(int fd)
	{
		int res;

		fd_set rfds;
		timeval timeout = { 1, 0 };
		FD_ZERO(&rfds);
		FD_SET(fd, &rfds);
		res = select(-1, &rfds, nullptr, nullptr, &timeout);

		if (res <= 0)
		{
			closesocket(fd);
			return false;
		}

		uchar buf[1024 * 16];
		auto ret = recv(fd, (char*)buf, array_size(buf), 0);

		auto p = buf;

		if (ret <= 0 || !(ret > 3 && p[0] == 'G' && p[1] == 'E' && p[2] == 'T'))
		{
			closesocket(fd);
			return false;
		}

		p[ret] = 0;

		std::regex reg_key(R"(Sec-WebSocket-Key: (.*))");

		std::string req((char*)p);
		auto lines = SUS::split(req, '\n');
		for (auto& l : lines)
		{
			std::smatch res;
			if (std::regex_search(l, res, reg_key))
			{
				std::string key = res[1];
				SHA1 sha1;
				sha1.update(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
				key = base64_encode(sha1.final_bin());

				char reply[1024 * 16], time_str[128];
				auto time = std::time(nullptr);
				std::strftime(time_str, array_size(time_str), "%a, %d %b %Y %FLAME_HASH:%M:%S GMT", std::localtime(&time));
				sprintf(reply, "HTTP/1.1 101 Switching Protocols\r\n"
					"Content-Length: 0\r\n"
					"Upgrade: websocket\r\n"
					"Sec-Websocket-Accept: %s\r\n"
					"Server: flame\r\n"
					"Connection: Upgrade\r\n"
					"Data: %s\r\n"
					"\r\n", key.c_str(), time_str);
				auto res = ::send(fd, reply, strlen(reply), 0);
				assert(res > 0);
			}
		}

		return true;
	}

	bool websocket_send(int fd, uint size, void* data)
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

		auto bytes = recv(fd, (char*)buf, array_size(buf), 0);
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

	void init()
	{
		static auto inited = false;
		if (!inited)
		{
			WSADATA wsa_d = {};
			WSAStartup(MAKEWORD(1, 1), &wsa_d);
			inited = true;
		}
	}

	struct ClientPrivate : Client
	{
		SocketType type;

		int fd;

		std::unique_ptr<Closure<void(void* c, const char* msg, uint size)>> on_message;
		std::unique_ptr<Closure<void(void* c)>> on_close;

		void* ev_ended;

		~ClientPrivate()
		{
			destroy_event(ev_ended);
		}

		void stop()
		{
			if (fd)
			{
				closesocket(fd);
				fd = 0;
			}
			on_close->call();
		}
	};

	void Client::send(void* data, uint size)
	{
		auto buf = new char[size + sizeof(uint)];
		*(uint*)buf = size;
		memcpy(buf + sizeof(uint), data, size);
		::send(((ClientPrivate*)this)->fd, buf, size + sizeof(uint), 0);
	}

	Client* Client::create(SocketType type, const char* ip, uint port, void on_message(void* c, const char* msg, uint size), void on_close(void* c), const Mail<>& capture)
	{
		init();

		int res;

		auto fd = socket(AF_INET, SOCK_STREAM, 0);
		assert(fd != INVALID_SOCKET);
		sockaddr_in address = {};
		address.sin_family = AF_INET;
		address.sin_addr.S_un.S_addr = inet_addr(ip);
		address.sin_port = htons(port);
		res = connect(fd, (sockaddr*)&address, sizeof(address));
		if (res == SOCKET_ERROR)
		{
			closesocket(fd);
			return nullptr;
		}

		auto c = new ClientPrivate;
		c->type = type;
		c->fd = fd;
		c->ev_ended = create_event(false, true);
		{
			auto _c = new Closure<void(void* c, const char* msg, uint size)>;
			_c->function = on_message;
			_c->capture = capture;
			c->on_message.reset(_c);
		}
		{
			auto _c = new Closure<void(void* c)>;
			_c->function = on_close;
			_c->capture = capture;
			c->on_close.reset(_c);
		}

		std::thread([=]() {
			while (true)
			{
				int size;
				auto ret = recv(fd, (char*)&size, sizeof(int), 0);
				if (ret < sizeof(int))
				{
					c->stop();
					set_event(c->ev_ended);
					return;
				}
				char buf[1024 * 64];
				auto p = buf;
				auto rest = size;
				while (rest > 0)
				{
					auto ret = recv(fd, p, rest, 0);
					if (ret <= 0)
					{
						c->stop();
						set_event(c->ev_ended);
						return;
					}
					p += ret;
					rest -= ret;
				}
				c->on_message->call(buf, size);
			}
		}).detach();

		return c;
	}

	void Client::destroy(Client* c)
	{
		auto thiz = (ClientPrivate*)c;
		thiz->stop();
		wait_event(thiz->ev_ended, -1);
		delete thiz;
	}

	struct ServerPrivate : Server
	{
		struct Client
		{
			int fd;
			std::unique_ptr<Closure<void(void* c, const char* msg, uint size)>> on_message;
			std::unique_ptr<Closure<void(void* c)>> on_close;

			void* ev_ended;

			void stop()
			{
				if (fd)
				{
					closesocket(fd);
					fd = 0;
					on_close->call();
				}
			}
		};

		struct DgramAddress
		{
			int fd;
			sockaddr* paddr;
		};

		SocketType type;

		int fd_d;
		int fd_s;

		std::vector<std::unique_ptr<Client>> cs;

		std::unique_ptr<Closure<void(void* c, void* id, const char* msg, uint size)>> on_dgram;
		std::unique_ptr<Closure<void(void* c, void* id)>> on_connect;

		void* ev_ended_d;
		void* ev_ended_s;

		void stop()
		{
			if (fd_d)
			{
				closesocket(fd_d);
				fd_d = 0;
			}
			if (fd_s)
			{
				closesocket(fd_s);
				fd_s = 0;
			}
			for (auto& c : cs)
				c->stop();
		}

		void remove_client(Client* c)
		{
			c->stop();
			wait_event(c->ev_ended, -1);
			for (auto it = cs.begin(); it != cs.end(); it++)
			{
				if (it->get() == c)
				{
					cs.erase(it);
					break;
				}
			}
		}
	};

	void Server::set_client(void* id, void on_message(void* c, const char* msg, uint size), void on_close(void* c), const Mail<>& capture)
	{
		{
			auto c = new Closure<void(void* c, const char* msg, uint size)>;
			c->function = on_message;
			c->capture = capture;
			((ServerPrivate::Client*)id)->on_message.reset(c);
		}
		{
			auto c = new Closure<void(void* c)>;
			c->function = on_close;
			c->capture = capture;
			((ServerPrivate::Client*)id)->on_close.reset(c);
		}
	}

	void Server::send(void* id, void* data, uint size, bool is_dgram)
	{
		if (!is_dgram)
		{
			auto buf = new char[size + sizeof(uint)];
			*(uint*)buf = size;
			memcpy(buf + sizeof(uint), data, size);
			::send(((ServerPrivate::Client*)id)->fd, buf, size + sizeof(uint), 0);
		}
		else
		{
			auto& da = *(ServerPrivate::DgramAddress*)id;
			sendto(da.fd, (char*)data, size, 0, da.paddr, sizeof(sockaddr_in));
		}
	}

	Server* Server::create(SocketType type, uint port, void on_dgram(void* c, void* id, const char* msg, uint size), void on_connect(void* c, void* id), const Mail<>& capture)
	{
		init();

		int res;
		sockaddr_in address;

		auto fd_d = socket(AF_INET, SOCK_DGRAM, 0);
		assert(fd_d != INVALID_SOCKET);
		address = {};
		address.sin_family = AF_INET;
		address.sin_addr.S_un.S_addr = INADDR_ANY;
		address.sin_port = htons(port);
		res = bind(fd_d, (sockaddr*)&address, sizeof(address));
		assert(res == 0);

		auto fd_s = socket(AF_INET, SOCK_STREAM, 0);
		assert(fd_s != INVALID_SOCKET);
		address = {};
		address.sin_family = AF_INET;
		address.sin_addr.S_un.S_addr = INADDR_ANY;
		address.sin_port = htons(port);
		res = bind(fd_s, (sockaddr*)&address, sizeof(address));
		assert(res == 0);
		res = listen(fd_s, 1);
		assert(res == 0);

		auto s = new ServerPrivate;
		s->type = type;
		s->fd_d = fd_d;
		s->fd_s = fd_s;
		s->ev_ended_d = create_event(false, true);
		s->ev_ended_s = create_event(false, true);
		{
			auto c = new Closure<void(void* c, void* id, const char* msg, uint size)>;
			c->function = on_dgram;
			c->capture = capture;
			s->on_dgram.reset(c);
		}
		{
			auto c = new Closure<void(void* c, void* id)>;
			c->function = on_connect;
			c->capture = capture;
			s->on_connect.reset(c);
		}

		std::thread([=]() {
			while (true)
			{
				char buf[1024 * 64];
				sockaddr_in address;
				int address_size = sizeof(address);
				auto res = recvfrom(s->fd_d, buf, sizeof(buf), 0, (sockaddr*)&address, &address_size);
				if (res <= 0)
				{
					closesocket(s->fd_d);
					s->fd_d = 0;
					set_event(s->ev_ended_d);
					return;
				}
				ServerPrivate::DgramAddress da;
				da.fd = s->fd_d;
				da.paddr = (sockaddr*)&address;
				s->on_dgram->call(&da, buf, res);
			}
		}).detach();

		std::thread([=]() {
			while (true)
			{
				auto fd = accept(s->fd_s, nullptr, nullptr);
				if (fd == INVALID_SOCKET)
				{
					s->stop();
					set_event(s->ev_ended_s);
					return;
				}
				auto c = new ServerPrivate::Client;
				c->fd = fd;
				c->ev_ended = create_event(false, true);
				s->on_connect->call(c);
				if (c->on_message || c->on_close)
					s->cs.emplace_back(c);
				else
				{
					closesocket(fd);
					delete c;
					continue;
				}

				std::thread([=]() {
					while (true)
					{
						int size;
						auto ret = recv(c->fd, (char*)&size, sizeof(int), 0);
						if (ret < sizeof(int))
						{
							s->remove_client(c);
							return;
						}
						char buf[1024 * 64];
						auto p = buf;
						auto rest = size;
						while (rest > 0)
						{
							auto ret = recv(c->fd, p, rest, 0);
							if (ret <= 0)
							{
								s->remove_client(c);
								return;
							}
							p += ret;
							rest -= ret;
						}
						c->on_message->call(buf, size);
					}
				}).detach();
			}
		}).detach();

		return s;
	}

	void Server::destroy(Server* s)
	{
		auto thiz = (ServerPrivate*)s;
		thiz->stop();
		wait_event(thiz->ev_ended_d, -1);
		wait_event(thiz->ev_ended_s, -1);
		for (auto& c : thiz->cs)
			wait_event(c->ev_ended, -1);
		delete thiz;
	}

	struct FrameSyncServerPrivate : FrameSyncServer
	{
		SocketType type;

		int frame;
		int semaphore;

		std::vector<int> fd_cs;

		nlohmann::json frame_data;

		void* ev_ended;

		~FrameSyncServerPrivate()
		{
			destroy_event(ev_ended);
		}

		static bool send_detail(SocketType type, int fd, void* data, int size)
		{
			if (type == SocketWeb)
				return websocket_send(fd, size, data);
			else
				return ::send(fd, (char*)data, size, 0) > 0;
		}

		bool send(uint client_id, void* data, int size)
		{
			return send_detail(type, fd_cs[client_id], data, size);
		}

		void stop()
		{
			for (auto& fd : fd_cs)
			{
				if (fd)
				{
					closesocket(fd);
					fd = 0;
				}
			}
		}
	};

	FrameSyncServer* FrameSyncServer::create(SocketType type, uint port, uint client_count)
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
			auto fd = accept(fd_s, nullptr, nullptr);
			if (fd == INVALID_SOCKET)
				return nullptr;

			if (type == SocketWeb)
			{
				if (!websocket_shakehank(fd))
					continue;
			}

			fd_cs.push_back(fd);
		}

		closesocket(fd_s);

		{
			srand(time(0));
			nlohmann::json json = {
				{"action", "start"},
				{"seed", ::rand()}
			};
			auto str = json.dump();
			for (auto fd : fd_cs)
			{
				if (!FrameSyncServerPrivate::send_detail(type, fd, str.data(), str.size()))
					return nullptr;
			}
		}

		auto s = new FrameSyncServerPrivate;
		s->type = type;
		s->frame = 0;
		s->semaphore = 0;
		s->fd_cs = fd_cs;
		s->ev_ended = create_event(false, true);

		std::thread([s]() {
			while (true)
			{
				fd_set rfds;
				FD_ZERO(&rfds);
				for (auto fd : s->fd_cs)
					FD_SET(fd, &rfds);
				auto res = select(-1, &rfds, nullptr, nullptr, nullptr);
				if (res < 0)
				{
					s->stop();
					set_event(s->ev_ended);
					return;
				}
				for (auto i = 0; i < s->fd_cs.size(); i++)
				{
					auto fd = s->fd_cs[i];
					if (FD_ISSET(fd, &rfds))
					{
						auto closed = false;
						auto reqs = websocket_recv(fd, closed);
						if (closed || reqs.empty())
						{
							s->stop();
							set_event(s->ev_ended);
							return;
						}
						auto req = nlohmann::json::parse(reqs[0]);
						auto n_frame = req.find("frame");
						if (n_frame != req.end())
						{
							auto frame = n_frame->get<int>();
							if (frame == s->frame)
							{
								s->semaphore++;
								auto dst = s->frame_data[std::to_string(i)];
								for (auto& i : req["data"].items())
									dst[i.key()] = i.value();

								if (s->semaphore >= s->fd_cs.size())
								{
									s->frame++;
									s->semaphore = 0;

									s->frame_data["action"] = "frame";
									auto str = s->frame_data.dump();
									for (auto i = 0; i < 2; i++)
										s->send(i, str.data(), str.size());
									s->frame_data.clear();
								}
							}
						}
					}
				}
			}
		}).detach();

		return s;
	}

	bool FrameSyncServer::send(uint client_idx, void* data, uint size)
	{
		return ((FrameSyncServerPrivate*)this)->send(client_idx, data, size);
	}

	void FrameSyncServer::destroy(FrameSyncServer* s)
	{
		auto thiz = (FrameSyncServerPrivate*)s;
		thiz->stop();
		wait_event(thiz->ev_ended, -1);
		delete thiz;
	}

	void board_cast(uint port, void* data, uint size, uint _timeout, void on_message(void* c, const char* ip, const char* msg, uint size), const Mail<>& capture)
	{
		init();

		int res;

		auto fd = socket(AF_INET, SOCK_DGRAM, 0);
		assert(fd != INVALID_SOCKET); 
		auto attr = 1;
		setsockopt(fd, SOL_SOCKET, SO_BROADCAST, (char*)&attr, sizeof(attr));
		sockaddr_in address;
		address.sin_family = AF_INET;
		address.sin_addr.S_un.S_addr = inet_addr("255.255.255.255");
		address.sin_port = htons(port);

		sendto(fd, (char*)data, size, 0, (sockaddr*)&address, sizeof(address));

		std::thread([=]() {
			int res;

			while (true)
			{
				timeval timeout = { _timeout, 0 };
				fd_set rfds;
				FD_ZERO(&rfds);
				FD_SET(fd, &rfds);
				res = select(-1, &rfds, nullptr, nullptr, &timeout);
				if (res <= 0)
				{
					closesocket(fd);
					delete_mail(capture);
					return;
				}

				char buf[1024 * 64];
				sockaddr_in address;
				int address_size = sizeof(address);
				res = recvfrom(fd, buf, sizeof(buf), 0, (sockaddr*)&address, &address_size);
				if (res <= 0)
				{
					closesocket(fd);
					delete_mail(capture);
					return;
				}
				on_message(capture.p, inet_ntoa(address.sin_addr), buf, res);
			}
		}).detach();
	}
}
